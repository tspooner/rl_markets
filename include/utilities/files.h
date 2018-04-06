#ifndef FILES_H
#define FILES_H

#include <glob.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/stat.h>

using std::string;
using std::vector;

inline vector<string> glob(const string& pat)
{
    using namespace std;

    glob_t glob_result;
    glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);

    vector<string> paths;
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i)
        paths.push_back(string(glob_result.gl_pathv[i]));

    globfree(&glob_result);

    return paths;
}

inline bool check_exists(string path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
        return false;
    else
        return true;
}

vector<std::tuple<string, string, string>> get_file_sample(
    string md_dir,
    string tas_dir,
    vector<string> symbols) {

    vector<std::tuple<string, string, string>> all_files;
    for (auto s : symbols) {
        // Check existence of data directories:
        string md_s_dir(md_dir + "/" + s);
        string tas_s_dir(tas_dir + "/" + s);

        if (not check_exists(md_s_dir))
            throw runtime_error("No such directory: " + md_s_dir);

        if (not check_exists(tas_s_dir))
            throw runtime_error("No such directory: " + tas_s_dir);

        // Find all file paths:
        auto files = glob(md_s_dir + "/*.csv");

        for (auto f : files) {
            string tf = f;

            tf.replace(0, md_dir.size(), tas_dir);

            int loc = f.find("md_");
            if (loc != -1)
                tf.replace(loc+1, 2, "tas");
            else
                throw runtime_error("Unexpected file name: " + f);

            if (access(tf.c_str(), F_OK) != -1)
                all_files.push_back(std::make_tuple(s, f, tf));
        }
    }

    return all_files;
}

vector<std::tuple<string, string, string>> get_sample_window(
    string md_dir, string tas_dir, string symbol, vector<string> search_patterns) {

    vector<std::tuple<string, string, string>> samples;

    string md_s_dir(md_dir + "/" + symbol);
    string tas_s_dir(tas_dir + "/" + symbol);

    if (not check_exists(md_s_dir))
        throw runtime_error("No such directory: " + md_s_dir);

    if (not check_exists(tas_s_dir))
        throw runtime_error("No such directory: " + tas_s_dir);

    // Find all file paths:
    for (auto p : search_patterns) {
        auto md_files = glob(md_s_dir + "/*" + p + "*.csv");
        auto tas_files = glob(tas_s_dir + "/*" + p + "*.csv");

        if (md_files.size() != tas_files.size())
            throw runtime_error("No matching files for MD and TAS.");
        else
            samples.push_back(std::make_tuple(symbol, md_files[0], tas_files[0]));
    }

    return samples;
}

#endif
