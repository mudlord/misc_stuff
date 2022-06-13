#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <array>
#include <curl/curl.h>
#include "picojson.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace picojson;

const char* wis = " \t\n\r\f\v";
inline std::string& rtrim(std::string& s, const char* t = wis){
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}
inline std::string& ltrim(std::string& s, const char* t = wis){
    s.erase(0, s.find_first_not_of(t));
    return s;
}
// trim from both ends of string (right then left)
inline std::string& trim(std::string& s, const char* t = wis)
{
    return ltrim(rtrim(s, t), t);
}

std::string url_encode(const string &value)
{
  ostringstream escaped;
  escaped.fill('0');
  escaped << hex;

  for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
  {
    string::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '=' ||
        c == '?' || c == ':' || c == '-' || c == '_' || c == '.' || c == '~' || c == '/')
    {
      escaped << c;
      continue;
    }
    else if (c == '\"')
    {
      escaped << "%22";
      continue;
    }

    // Any other characters are percent-encoded
    escaped << uppercase;
    escaped << '%' << setw(2) << int((unsigned char)c);
    escaped << nouppercase;
  }

  return escaped.str();
}



size_t json_callback(
    const char *in,
    size_t size,
    size_t num,
    string *out)
{
  const size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}

struct songdb {
    string dataentry;
    string artist;
    string songtitle;
    int songcnt;
};

void parse_file(string path, vector<songdb> &map)
{
  ifstream file;
  file.open(path);
  string segment;
  while (std::getline(file, segment))
  {
    
    string songtitle;
    string songtitle_real;
    string songcount;
    string songartist;
    size_t idx_dash = segment.find('-');
    size_t idx2 = segment.find_last_of('"');
    songartist = segment.substr(1,idx_dash-1);
    songartist = trim(songartist);
    songtitle = segment.substr(1, idx2 - 1);
    songtitle = trim(songtitle);
    songtitle_real = segment.substr(idx_dash+1,(idx2)-(idx_dash+1));
    songtitle_real = trim(songtitle_real);
    songcount = segment.substr(idx2 + 2);
    int songcnt = stoi(songcount);

    songdb db={songtitle,songartist,songtitle_real,songcnt};

    auto check = [&db](const songdb& x) { return x.dataentry == db.dataentry;};
    auto it= std::find_if(map.begin(), map.end(),check);
    if(it == map.end())
    map.push_back(db);
    else
    (*it).songcnt += songcnt;
  }
}

int listproc(vector<songdb> &map)
{
  auto curl = std::unique_ptr<CURL, 
  decltype(&curl_easy_cleanup)>(curl_easy_init(), curl_easy_cleanup);

  if (!curl)
  {
    fprintf(stderr, "Error: curl_easy_init failed.\n");
    
    return 1;
  }

  ofstream songlists;
  songlists.open("songurls.txt", std::ios_base::trunc);

  for (auto &x : map)
  {
    //https://api.deezer.com/search?q=artist:"aloe blacc" track:"i need a dollar"
    string URLtoprobe = "https://api.deezer.com/search?q=artist:\"";
    URLtoprobe += x.artist +"\"" + " track:"+"\""+x.songtitle+"\"";
    URLtoprobe = url_encode(URLtoprobe);
    unique_ptr<string> json_data(new string());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, json_callback);
    curl_easy_setopt(curl.get(), CURLOPT_URL, URLtoprobe.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, json_data.get());
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:99.0) Gecko/20100101 Firefox/99.0");
    CURLcode res = curl_easy_perform(curl.get());
    chrono::seconds dura(2);
    this_thread::sleep_for(dura); // cheapass delay to work around api limits

    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      return 1;
    }
    else
    {

      picojson::value v;
      string err = picojson::get_last_error();
      parse(v, json_data.get()->begin(), json_data.get()->end(), &err);
      if (!err.empty())
        continue;

      if (!v.is<picojson::object>())
        continue;
      picojson::array list = v.get("data").get<picojson::array>();
      for (auto &iter : list)
      {
        if (iter.get("title").get<string>()==x.songtitle);
          if (iter.get("artist").get("name").get<string>()==x.artist)
          {
            
            string songl = iter.get("link").get<string>();
            songlists << songl << std::endl;
            break;
          }
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[])
{
  std::vector<songdb> songmap;

  filesystem::path path = filesystem::current_path();
  for (auto &entry : filesystem::directory_iterator(path))
  {
    string str = entry.path().string();
    if (entry.is_regular_file() && entry.path().extension() == ".stdb")
      parse_file(entry.path().string(), songmap);
  }

  listproc(songmap);
  return 0;
}
