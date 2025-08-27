//  Copyright 2024 Omar El-Rifai

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

struct BootLog {
    int startLineNum;
    int endLineNum;
    std::string sourceLogFile;
    boost::posix_time::ptime bootStartTime;
    boost::posix_time::ptime bootEndTime;
    bool isBootComplete =  false;
};

boost::posix_time::ptime extractTimestamp(const std::string& logLine) {
    std::regex timestampRegex("(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})");
    std::smatch timestampMatch;
    if (std::regex_search(logLine, timestampMatch, timestampRegex)) {
        return boost::posix_time::time_from_string(timestampMatch[1]);
    }
    return boost::posix_time::not_a_date_time;
}

std::string formatDateTime(const boost::posix_time::ptime& timeStamp) {
    std::ostringstream formattedStream;
    auto* timeFacet = new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S");
    formattedStream.imbue(std::locale(formattedStream.getloc(), timeFacet));
    formattedStream << timeStamp;
    return formattedStream.str();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <log_file_path>\n";
        return 1;
    }

    std::string inputLogFile = argv[1];
    std::string reportFile = inputLogFile + ".rpt";
    std::ifstream inputFile(inputLogFile);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open log file: " << inputLogFile << std::endl;
        return 1;
    }

    std::ofstream outputFile(reportFile);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to create output file: " << reportFile << std::endl;
        return 1;
    }

    std::string currentLine;
    std::vector<BootLog> bootRecords;
    int scannedLines = 0;

    while (getline(inputFile, currentLine)) {
        scannedLines++;
        if (currentLine.find("(log.c.166) server started") != std::string::npos) {
            BootLog newBoot;
            newBoot.startLineNum = scannedLines;
            newBoot.bootStartTime = extractTimestamp(currentLine);
            newBoot.sourceLogFile = inputLogFile;
            if (newBoot.bootStartTime.is_not_a_date_time()) {
                std::cerr << "Failed to parse start timestamp on line: " << scannedLines << "\n";
            } else {
                bootRecords.push_back(newBoot);
            }
        } else if (currentLine.find("SelectChannelConnector@0.0.0.0:9080") != std::string::npos) {
            if (!bootRecords.empty() && !bootRecords.back().isBootComplete) {
                bootRecords.back().endLineNum = scannedLines;
                bootRecords.back().bootEndTime = extractTimestamp(currentLine);
                if (bootRecords.back().bootEndTime.is_not_a_date_time()) {
                    std::cerr << "Failed to parse end timestamp on line: " << scannedLines << "\n";
                } else {
                    bootRecords.back().isBootComplete = true;
                }
            }
        }
    }

    for (const auto& bootRecord : bootRecords) {
        if (bootRecord.isBootComplete) {
            boost::posix_time::time_duration bootDuration =
            bootRecord.bootEndTime - bootRecord.bootStartTime;
            outputFile << "=== Device boot ===\n";
            outputFile << bootRecord.startLineNum << "(" << inputLogFile << "): ";
            outputFile << formatDateTime(bootRecord.bootStartTime) << " Boot Start\n";
            outputFile << bootRecord.endLineNum << "(" << inputLogFile << "): "
                       << formatDateTime(bootRecord.bootEndTime) << " Boot Completed\n";
            outputFile << "\tBoot Time: " << bootDuration.total_milliseconds() << "ms\n\n";
        } else {
            outputFile << "=== Device boot ===\n";
            outputFile << bootRecord.startLineNum << "(" << inputLogFile << "): ";
            outputFile << formatDateTime(bootRecord.bootStartTime) << " Boot Start\n";
            outputFile << "**** Incomplete boot ****" << "\n\n";
        }
    }

    inputFile.close();
    outputFile.close();
    return 0;
}
