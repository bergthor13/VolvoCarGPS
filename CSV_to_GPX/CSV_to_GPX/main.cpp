//
//  main.cpp
//  NewGPSToGPX
//
//  Created by Bergþór on 27.12.2014.
//  Copyright (c) 2014 Bergþór Þrastarson. All rights reserved.
//

#include <iostream>
#include <vector>

#include <fstream>

using namespace std;

struct GPSPoint {
    string time;
    double latitude;
    double longitude;
    double altitude;
    double temperature;
    
    bool equals (GPSPoint that) {
        return this->latitude  == that.latitude &&
        this->longitude == that.longitude;
        
    }
};

GPSPoint parseLine(string line) {
    GPSPoint p;
    size_t nextComma = 0, pointer = 0;
    string delimiter = ";";
    
    nextComma = line.find(delimiter, pointer);
    p.time = line.substr(pointer, nextComma-pointer);
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    p.latitude = atof(line.substr(pointer, nextComma-pointer).c_str());
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    p.longitude = atof(line.substr(pointer, nextComma-pointer).c_str());
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    p.altitude = atof(line.substr(pointer, nextComma-pointer).c_str());
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    //cout << line[pointer] << endl;
    if (line[pointer] == 'N') {
        
        p.temperature = numeric_limits<double>::min();
    } else {
        p.temperature = atof(line.substr(pointer, nextComma-pointer).c_str());
    }
    
    pointer = nextComma + 1;
    
    return p;
}

void logHeader(ofstream &f) {
    f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    f << "<gpx version=\"1.1\" " << endl;
    f << "     creator=\"Garmin Connect\"" << endl;
    f << "     xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1" << endl;
    f << "                         http://www.topografix.com/GPX/1/1/gpx.xsd" << endl;
    f << "                         http://www.garmin.com/xmlschemas/GpxExtensions/v3" << endl;
    f << "                         http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd" << endl;
    f << "                         http://www.garmin.com/xmlschemas/TrackPointExtension/v1" << endl;
    f << "                         http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\"" << endl;
    f << "     xmlns=        \"http://www.topografix.com/GPX/1/1\"" << endl;
    f << "     xmlns:gpxtpx= \"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\"" << endl;
    f << "     xmlns:gpxx=   \"http://www.garmin.com/xmlschemas/GpxExtensions/v3\"" << endl;
    f << "     xmlns:xsi=    \"http://www.w3.org/2001/XMLSchema-instance\">" << endl;
    f << "    <trk>" << endl;
    f << "        <name>RH508</name>" << endl;
    f << "        <trkseg>" << endl;
}

void logToFile(GPSPoint p, ofstream &f) {
    f << "            <trkpt lon=\""<< p.longitude << "\" lat=\"" << p.latitude << "\">" << endl;
    f << "                <ele>" << p.altitude << "</ele>" << endl;
    f << "                <time>" << p.time << "</time>" << endl;
    
    if (!(p.temperature == numeric_limits<double>::min())) {
        f << "                <extensions>" << endl;
        f << "                    <gpxtpx:TrackPointExtension>" << endl;
        f << "                    <gpxtpx:atemp>" << p.temperature << "</gpxtpx:atemp>" << endl;
        f << "                    </gpxtpx:TrackPointExtension>" << endl;
        f << "                </extensions>" << endl;
    }

    f << "            </trkpt>" << endl;
    
}

void logFooter(ofstream &f) {
    f << "        </trkseg>" << endl;
    f << "    </trk>" << endl;
    f << "</gpx>" << endl;
}



int main(int argc, const char * argv[]) {
    string line, fileLoc = "/Volumes/UNTITLED/LOG0000.CSV";
    GPSPoint now, old;
    ifstream csvFile(fileLoc);
    ofstream gpxFile;

    
    gpxFile.setf(ios::fixed);
    gpxFile.precision(14);
    gpxFile.open("HELLOTHERE5.gpx");
    
    logHeader(gpxFile);
    while (csvFile.good()) {
        getline(csvFile, line);
        now = parseLine(line);
        if ((!now.equals(old) && line != "") && (now.time != old.time)) {
            logToFile(now, gpxFile);
        }
        old = now;
    }
    logFooter(gpxFile);
    
    return 0;
}