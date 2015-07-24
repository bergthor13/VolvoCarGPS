//
//  main.cpp
//  NewGPSToGPX
//
//  Created by Bergþór on 27.12.2014.
//  Copyright (c) 2014 Bergþór Þrastarson. All rights reserved.
//

#include <iostream>
#include <vector>
#include <iomanip>

#include <fstream>

using namespace std;

inline bool exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

struct Date {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
    
    Date(string d) {
        if (d.length() > 20) {
            size_t milliP = d.find("Z");
            size_t timeP = d.find("T");
            year =        stoi(d.substr(0, 4));
            month =       stoi(d.substr(5, 2));
            day =         stoi(d.substr(8, 2));
            hour =        stoi(d.substr(timeP+1, 2));
            minute =      stoi(d.substr(timeP+4, 2));
            second =      stoi(d.substr(timeP+7, 2));
            millisecond = stoi(d.substr(timeP+10, 20-milliP));
            if (millisecond >= 500) addSecond();
        }
    }
    
    void addSecond() {
        if (isLastDayOfYear() && hour == 23 && minute == 59 && second == 59) {
            year++; month = 1; day = 1; hour = 0; minute = 0; second = 0;
            return;
        }
        
        if (isLastDayOfMonth() && hour == 23 && minute == 59 && second == 59) {
            month++; day = 1; hour = 0; minute = 0; second = 0;
            return;
        }
        
        second++;
        
        if (second == 60) {second = 0; minute++;}
        if (minute == 60) {minute = 0; hour++;}
        if (hour   == 24) {day++; hour = 0; minute = 0; second = 0;}
    }
    
    bool isLastDayOfMonth() {
        if      (month == 1  && day == 31)                  return true;
        else if (month == 2  && day == 28 && !isLeapYear()) return true;
        else if (month == 2  && day == 29 && isLeapYear())  return true;
        else if (month == 3  && day == 31)                  return true;
        else if (month == 4  && day == 30)                  return true;
        else if (month == 5  && day == 31)                  return true;
        else if (month == 6  && day == 30)                  return true;
        else if (month == 7  && day == 31)                  return true;
        else if (month == 8  && day == 31)                  return true;
        else if (month == 9  && day == 30)                  return true;
        else if (month == 10 && day == 31)                  return true;
        else if (month == 11 && day == 30)                  return true;
        else if (month == 12 && day == 31)                  return true;
        
        return false;
    }
    
    bool isLastDayOfYear() {
        return month == 12 && day == 31;
    }
    
    bool isLeapYear() {
        bool leapYear = false;
        if (year%4 == 0) {
            leapYear = true;
            if (year%100 == 0) {
                if (year%400 != 0) {
                    leapYear = false;
                }
            }
        } else {
            leapYear = false;
        }
        return leapYear;
    }
    string toString() {
        return to_string(year) + "-" +
        (month  < 10 ? to_string(0) : "") + to_string(month)  + "-" +
        (day    < 10 ? to_string(0) : "") + to_string(day)    + "T" +
        (hour   < 10 ? to_string(0) : "") + to_string(hour)   + ":" +
        (minute < 10 ? to_string(0) : "") + to_string(minute) + ":" +
        (second < 10 ? to_string(0) : "") + to_string(second) + ".000Z";
    }
    string toFileString() {
        return to_string(year) + "." +
        (month  < 10 ? to_string(0) : "") + to_string(month)  + "." +
        (day    < 10 ? to_string(0) : "") + to_string(day)    + "-" +
        (hour   < 10 ? to_string(0) : "") + to_string(hour)   +
        (minute < 10 ? to_string(0) : "") + to_string(minute) +
        (second < 10 ? to_string(0) : "") + to_string(second);
    }
    
    bool equals (Date that) {
        return this->year        == that.year &&
               this->month       == that.month &&
               this->day         == that.day &&
               this->hour        == that.hour &&
               this->minute      == that.minute &&
               this->second      == that.second &&
               this->millisecond == that.millisecond;
        
    }
};


struct GPSPoint {
    Date*  time;
    double latitude;
    double longitude;
    double altitude;
    double temperature;
    
    bool equals (GPSPoint that) {
        return this->latitude  == that.latitude &&
        this->longitude == that.longitude;
        
    }
    GPSPoint(string t) {
        time = new Date(t);
    }
};

GPSPoint parseLine(string line) {
    size_t nextComma = 0, pointer = 0;
    string delimiter = ";";
    
    nextComma = line.find(delimiter, pointer);
    GPSPoint p(line.substr(pointer, nextComma-pointer));
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
    f << "                <time>" << p.time->toString() << "</time>" << endl;
    
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
    GPSPoint now(""), old("");
    ifstream csvFile;
    ofstream gpxFile;
    
    
    gpxFile.setf(ios::fixed);
    gpxFile.precision(14);
    
    int i = 0;
    while (true) {
        double avgTemp = 0,
               maxTemp = numeric_limits<double>::min(),
               minTemp = numeric_limits<double>::max();
        size_t points = 0;
        bool firstIteration = true;
        size_t locOf00 = fileLoc.find("LOG");
        fileLoc[locOf00+3] = '0' + (i / 1000) % 10;
        fileLoc[locOf00+4] = '0' + (i / 100) % 10;
        fileLoc[locOf00+5] = '0' + (i / 10) % 10;
        fileLoc[locOf00+6] = '0' + (i % 10);
        if (!exists(fileLoc)) {
            break;
        }
        csvFile.open(fileLoc);
        while (csvFile.good()) {
            getline(csvFile, line);
            // If line is empty line.
            if (line != "") now = parseLine(line); else {csvFile.close(); break;}
            
            if (firstIteration) {
                firstIteration = false;
                gpxFile.open("/Users/Bergthor/Dropbox/Apps/WahooFitness/NJ578/" + now.time->toFileString() + ".gpx");
                logHeader(gpxFile);
            }
            
            if ((!now.equals(old) && line != "") && (!now.time->equals(*old.time))) {
                avgTemp+= now.temperature;
                logToFile(now, gpxFile);
            }
            old = now;
            points++;
        }
        
        logFooter(gpxFile);
        cout.setf(ios::fixed);
        cout.precision(2);
        if (points > 0)
            cout << setw(4) << points << setw(10) << avgTemp / points << endl;
        firstIteration = true;
        gpxFile.close();
        csvFile.close();
        i++;
    }
    
    return 0;
}