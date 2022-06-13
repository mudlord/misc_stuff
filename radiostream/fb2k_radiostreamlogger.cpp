#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
using namespace pfc;
using namespace std;
/*
Compile using the FB2K SDK.
Based around the foobar2000 sample component.

Should be easy enough to go from there.
Uses C++17 because why the fuck not!

--mudlord
*/
namespace {
	

	class play_callback_psc : public play_callback_static {
		std::map<std::string, int> songmap;
	private:
		void song_changed(pfc::string songtit)
		{
			std::filesystem::path path = std::filesystem::current_path() / "stream.txt";
			ofstream file_out;
			std::map<std::string, int>::iterator it = songmap.find(songtit.c_str());
			if (it == songmap.end())
				songmap.emplace(songtit.c_str(), 1);
			else
				++songmap[songtit.c_str()];
			file_out.open(path, std::ios_base::binary);
			for (auto& x : songmap)
			file_out << '"' << x.first << '"' << "," << x.second << endl;
		}
	public:
		void on_playback_dynamic_info_track(const file_info& p_info)
		{
			const char* artist = p_info.meta_get("artist", 0);
			if (artist == NULL)return;
			const char* title = p_info.meta_get("title", 0);
			if (title == NULL)return;
			pfc::string art = artist;
			pfc::string title1 = title;
			pfc::string songtit = art + " - " + title1;
			if (!art.is_empty())
				song_changed(songtit);
		}
		unsigned get_flags() override {
			return flag_on_playback_dynamic_info_track | flag_on_playback_starting |
				flag_on_playback_stop;
		}
		void on_playback_stop(play_control::t_stop_reason p_reason) override {
			// Terminate the current stream
		}
		void on_playback_starting(play_control::t_track_command p_command, bool p_paused) override {
			
		}
		void on_playback_new_track(metadb_handle_ptr p_track) override {

		}
		void on_playback_seek(double p_time) override {}
		void on_playback_pause(bool p_state) override {}
		void on_playback_edited(metadb_handle_ptr p_track) override {}
		void on_playback_dynamic_info(const file_info& p_info) override {
		}
		
		void on_playback_time(double p_time) override {}
		void on_volume_change(float p_new_val) override {}
	};
	FB2K_SERVICE_FACTORY(play_callback_psc);
}