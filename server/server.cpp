// Name: Jacob Feng
// SID: 1591656
// CCID: jfeng4
// CMPUT 275 Wi22
//
// Assignment 2: Navigation System
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <list>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "wdigraph.h"
#include "dijkstra.h"

struct Point {
    long long lat, lon;
};

// returns the manhattan distance between two points
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}

// finds the id of the point that is closest to the given point "pt"
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}

// read the graph from the file that has the same format as the "Edmonton graph" file
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will be 4 substrings either way
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // start new string
        ++at;
      }
      else {
        // append character to the string we are building
        p[at] += c;
      }
    }

    if (at != 3) {
      // empty line
      break;
    }

    if (p[0] == "V") {
      // new Point
      int id = stoi(p[1]);
      assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit
      points[id].lat = static_cast<long long>(stod(p[2])*100000);
      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    }
    else {
      // new directed edge
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));
    }
  }
}

int create_and_open_fifo(const char * pname, int mode) {
  // creating a fifo special file in the current working directory
  // with read-write permissions for communication with the plotter
  // both proecsses must open the fifo before they can perform
  // read and write operations on it
  if (mkfifo(pname, 0666) == -1) {
    cout << "Unable to make a fifo. Ensure that this pipe does not exist already!" << endl;
    exit(-1);
  }

  // opening the fifo for read-only or write-only access
  // a file descriptor that refers to the open file description is
  // returned
  int fd = open(pname, mode);

  if (fd == -1) {
    cout << "Error: failed on opening named pipe." << endl;
    exit(-1);
  }

  return fd;
}

// keep in mind that in part 1, the program should only handle 1 request
// in part 2, you need to listen for a new request the moment you are done
// handling one request
int main() {
  WDigraph graph;
  unordered_map<int, Point> points;

  const char *inpipe = "inpipe";
  const char *outpipe = "outpipe";

  // Open the two pipes
  int in = create_and_open_fifo(inpipe, O_RDONLY);
  cout << "inpipe opened..." << endl;
  int out = create_and_open_fifo(outpipe, O_WRONLY);
  cout << "outpipe opened..." << endl;  

  // build the graph
  readGraph("server/edmonton-roads-2.0.1.txt", graph, points);

  // read a request
  int byteSize;
  int spaceIndex, newlineIndex;

  // Keep listening for requests
  while (true) {
    Point sPoint, ePoint;
    char buffer[1024];

    // Read from inpipe and convert to string
    byteSize = read(in, buffer, 1024);
    string inString(buffer);

    // Close program if client done sending requests
    if (inString[0] == 'Q') {
      cout << "Shutting down..." << endl;
      break;
    }

    // newlineIndex is 
    spaceIndex = inString.find(" ");
    newlineIndex = inString.find("\n");

    // Get starting coordinates, split them by their space
    sPoint.lat = stod(inString.substr(0, spaceIndex)) * 100000;
    sPoint.lon = stod(inString.substr(spaceIndex, newlineIndex)) * 100000;

    // Debugging
    cout << "buffer: " << buffer << endl;
    cout << "Starting latitude: " << sPoint.lat << endl;
    cout << "Starting longitude: " << sPoint.lon << endl;
    
    // Get ending coordinates starting from the newline, split them by the second space
    spaceIndex = inString.find(" ", newlineIndex);
    ePoint.lat = stod(inString.substr(newlineIndex, spaceIndex)) * 100000;
    ePoint.lon = stod(inString.substr(spaceIndex)) * 100000;

    // Debugging
    cout << "Ending latitude: " << ePoint.lat << endl;
    cout << "Ending longitude: " << ePoint.lon << endl;

    // get the points closest to the two points we read
    int start = findClosest(sPoint, points), end = findClosest(ePoint, points);

    // run dijkstra's algorithm, this is the unoptimized version that
    // does not stop when the end is reached but it is still fast enough
    unordered_map<int, PIL> tree;
    dijkstra(graph, start, tree);

    string deciLat, deciLon, strBuffer = "";

    // no path
    if (tree.find(end) == tree.end()) {
        cout << "N 0" << endl;
    }
    else {
      // read off the path by stepping back through the search tree
      list<int> path;
      while (end != start) {
        path.push_front(end);
        end = tree[end].first;
      }
      path.push_front(start);

      // output the path
      cout << "N " << path.size() << endl;

      for (int v : path) {
        // Obtain decimal point string repr of coords
        deciLat = to_string(static_cast<double>(points[v].lat) / 100000);
        deciLon = to_string(static_cast<double>(points[v].lon) / 100000);

        // Remove trailing zeros
        deciLat.pop_back();
        deciLon.pop_back();

        // Append line to buffer
        strBuffer += deciLat + " " + deciLon + "\n";
      }
    }

    // Append end signal
    strBuffer += "E\n";
      
    // write to outpipe
    write(out, strBuffer.data(), strBuffer.size());
  }


  // Close and unlink from named pipes
  close(in);
  close(out);
  unlink(inpipe);
  unlink(outpipe);

  return 0;
}
