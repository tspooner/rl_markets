#ifndef UTILITIES_CSV_H
#define UTILITIES_CSV_H

#include <vector>
#include <fstream>

using namespace std;

class CSV
{
    private:
        ifstream fs;

        void parseRow(string& line, vector<string>& cols);

    public:
        CSV() = default;
        CSV(const string& file_path);


        // File control methods
        void openFile(const string& file_path);
        void closeFile();

        bool isOpen();
        bool hasData();

        void next(vector<string>& columns);
        void peek(vector<string>& columns);
        void skip(int n_lines = 1);
};

#endif
