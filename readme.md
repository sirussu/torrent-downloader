# Torrent Downloader Plus Plus

> little tool for downloading torrent for sirus.su launcher

#### input info

```bash

#usage
binary  <command> <parameters>

#command "torrent"
<torrent-file w path> <download-dir full path> <speed limit bytes>

# torrent-file - full path to the *.torrent file
# download-dir - full path to the dir where torrent was downloaded (optional, default current dir)
# speed limit  - if sets, downloaded speed will be limited

```
#### output info

### update message

```json
{ "message" : "%s", "error" : %b, "type" : "%s" }
```

For errors we have four types (error = 1)
1) download-error
message - torrent_error_alert.message() from libtorrent lib
2) boost-error
message - message for boost error codes
3) libtorrent-error
message - code for libtorrent lib errors, they not have descriptions
4) system-error
message - message for default system wide exceptions

In not error cases we got message is equal to torrent file, and different types: 
1) download-setup - when we trying to add torrent
2) download-started - when we successful starting downloading
3) download-done - when we successful downloaded all files

### status message

```json
{ "updated_at" : %llu, "bytes" : %llu, "bytes_done" : %llu, "progress" : %.2f, "type" : "%s", "speed" : %.2f }
```

updated_at - last timestamp when this message was invoked
bytes - total size of files in bytes
bytes_done - downloaded files size in bytes
progress - percent of files download
speed - current speed in bytes
type - one of these values:
```cpp
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
```
or "progress" in any other cases
