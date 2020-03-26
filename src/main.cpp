#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include <libtorrent/entry.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/error_code.hpp>
#include "StringFormat.h"

using clk = std::chrono::steady_clock;

// return the name of a torrent status enum
char const* state(lt::torrent_status::state_t s)
{
	switch (s) {
		case lt::torrent_status::checking_files: return "checking";
		case lt::torrent_status::downloading_metadata: return "dl metadata";
		case lt::torrent_status::downloading: return "progress";
		case lt::torrent_status::finished: return "finished";
		case lt::torrent_status::seeding: return "seeding";
		case lt::torrent_status::allocating: return "allocating";
		case lt::torrent_status::checking_resume_data: return "checking resume";
		default: return "<>";
	}
}

const std::string UpdateJson = R"({"message" : "%s", "error" : %b, "type" : "%s"})";
const std::string StatusJson = R"({"updated_at" : %llu, "bytes" : %llu, "bytes_done" : %llu, "progress" : %.5f, "type" : "%s", "speed" : %llu})";

void OutStatus(std::string text)
{
	std::cout << text << std::endl << std::flush;
}

int main(int argc, char const* argv[])
{
	if (argc < 2) {
		std::cerr << "usage: " << argv[0] << " <command> <parameters>" << std::endl;
		return 1;
	}

	std::string command(argv[1]);

	if (command == "torrent")
	{
		std::string filename = argv[2];
		std::string saveDir = "";
		if (argc > 3)
			saveDir = argv[2];
		int64_t speedLimit = 0;
		if (argc > 4)
			speedLimit = atoi(argv[3]);

		try
		{
			lt::settings_pack pack;
			pack.set_int(lt::settings_pack::alert_mask
				, lt::alert::error_notification
				| lt::alert::storage_notification
				| lt::alert::status_notification);
			pack.set_bool(lt::settings_pack::enable_dht, true);
			pack.set_bool(lt::settings_pack::enable_lsd, true);
			pack.set_int(lt::settings_pack::in_enc_policy, 1);
			pack.set_int(lt::settings_pack::out_enc_policy, 1);
			pack.set_int(lt::settings_pack::stop_tracker_timeout, 1);
			pack.set_str(lt::settings_pack::user_agent, "Sirus/Launcher");
			pack.set_str(lt::settings_pack::dht_bootstrap_nodes, "router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881,dht.aelitis.com:6881");


			lt::session ses(pack);
			clk::time_point last_save_resume = clk::now();
			lt::add_torrent_params atp;
			lt::add_torrent_params ntp;
			ntp.ti = std::make_shared<lt::torrent_info>(filename);

			// load resume data from disk and pass it in as we add the magnet link
			std::ifstream ifs(".resume_file", std::ios_base::binary);
			if (ifs.good())
			{
				ifs.unsetf(std::ios_base::skipws);
				std::vector<char> buf{ std::istream_iterator<char>(ifs)
				  , std::istream_iterator<char>() };

				atp = lt::read_resume_data(buf);
				if (atp.info_hash != ntp.info_hash) {
					atp = std::move(ntp);
				}
			}
			else
				atp = std::move(ntp);

			atp.save_path = "."; // save in current dir
			if (!saveDir.empty())
				atp.save_path = saveDir;

			OutStatus(Trinity::StringFormat(UpdateJson, atp.ti->name(), false, "download-setup"));
			ses.async_add_torrent(std::move(atp));

			//this is the handle we'll set once we get the notification of it
			lt::torrent_handle h;
			for (;;) {
				std::vector<lt::alert*> alerts;
				ses.pop_alerts(&alerts);

				for (lt::alert const* a : alerts) {
					if (auto at = lt::alert_cast<lt::add_torrent_alert>(a)) {
						h = at->handle;
						if (speedLimit)
							h.set_download_limit(speedLimit);
						OutStatus(Trinity::StringFormat(UpdateJson, h.torrent_file()->name(), false, "download-started"));
					}
					// if we receive the finished alert or an error, we're done
					if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
						h.save_resume_data();
						OutStatus(Trinity::StringFormat(UpdateJson, h.torrent_file()->name(), false, "download-done"));
						return 0;
					}
					if (lt::alert_cast<lt::torrent_error_alert>(a)) {
						OutStatus(Trinity::StringFormat(UpdateJson, a->message(), true, "download-error"));
						return 0;
					}

					// when resume data is ready, save it
					if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a)) {
						std::ofstream of(".resume_file", std::ios_base::binary);
						of.unsetf(std::ios_base::skipws);
						auto const b = write_resume_data_buf(rd->params);
						of.write(b.data(), b.size());
					}

					if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {
						if (st->status.empty()) continue;

						// we only have a single torrent, so we know which one
						// the status is for
						lt::torrent_status const& s = st->status[0];
						/*std::cout << "\r" << state(s.state) << " "
							<< (s.download_payload_rate / 1000) << " kB/s "
							<< (s.total_done / 1000) << " kB ("
							<< (s.progress_ppm / 10000) << "%) downloaded\x1b[K";
						std::cout.flush();*/

						time_t updateAt = time(nullptr);

						OutStatus(Trinity::StringFormat(StatusJson, updateAt, s.total, s.total_done, s.progress, state(s.state), s.download_payload_rate));
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

				// ask the session to post a state_update_alert, to update our
				// state output for the torrent
				ses.post_torrent_updates();

				// save resume data once every 30 seconds
				if (clk::now() - last_save_resume > std::chrono::seconds(30)) {
					h.save_resume_data();
					last_save_resume = clk::now();
				}
			}

			// TODO: ideally we should save resume data here

			h.save_resume_data();
		}
		catch (boost::system::system_error const& e)
		{
			boost::system::error_code ec = e.code();
			if (ec.category() != lt::libtorrent_category())
				OutStatus(Trinity::StringFormat(UpdateJson, ec.message(), true, "boost-error"));
			else
				OutStatus(Trinity::StringFormat(UpdateJson, ec.value(), true, "libtorrent-error"));
		}
		catch (std::exception & e)
		{
			OutStatus(Trinity::StringFormat(UpdateJson, e.what(), true, "system-error"));
		}
	}

	return 0;
}
