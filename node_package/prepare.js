const { http, https } = require('follow-redirects');
const fs = require('fs');
const path = require('path');
const unzipper = require('unzipper')

const RELEASE = 'v1.0';

const executables = [
    `https://github.com/sirussu/torrent-downloader/releases/download/${RELEASE}/td-linux-x64.zip`,
    `https://github.com/sirussu/torrent-downloader/releases/download/${RELEASE}/td-win-x64.zip`,
    `https://github.com/sirussu/torrent-downloader/releases/download/${RELEASE}/td-win-x86.zip`,
];

(async function prepare() {
    await Promise.all(executables.map(url => {
        return new Promise((resolve, reject) => {
            const extract = unzipper.Extract({ path: `vendor/` });
            https.get(url, function(response) {
                response.pipe(extract);
                extract.on('close', resolve)
            }).on('error', function(err) {
                reject(err)
            });
        })
    }))
})();