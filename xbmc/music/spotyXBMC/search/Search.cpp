/*
 spotyxbmc2 - A project to integrate Spotify into XBMC
 Copyright (C) 2011  David Erenger

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For contact with the author:
 david.erenger@gmail.com
 */

#include "Search.h"
#include "../session/Session.h"
#include "../Logger.h"
#include "../XBMCUpdater.h"
#include "../Settings.h"
#include "SearchResultBackgroundLoader.h"
#include "../album/SxAlbum.h"
#include "../track/SxTrack.h"
#include "../artist/SxArtist.h"

namespace addon_music_spotify {

  Search::Search(string query) {
    m_maxArtistResults = Settings::getSearchNumberArtists();
    m_maxAlbumResults = Settings::getSearchNumberAlbums();
    m_maxTrackResults = Settings::getSearchNumberTracks();

    m_query = query;

    m_artistsDone = false;
    m_albumsDone = false;
    m_tracksDone = false;

    //do the initial search
    m_cancelSearch = false;
    Logger::printOut("creating search");
    Logger::printOut(query);
    m_currentSearch = sp_search_create(Session::getInstance()->getSpSession(), m_query.c_str(), 0, m_maxTrackResults, 0, m_maxAlbumResults, 0, m_maxArtistResults, &cb_searchComplete, this);

  }

  Search::~Search() {
    //we need to wait for the results
    //m_cancelSearch = true;
    //while (m_currentSearch != NULL)
    //  ;
    Logger::printOut("cleaning after search");
    while (!m_tracks.empty()) {
      TrackStore::getInstance()->removeTrack(m_tracks.back());
      m_tracks.pop_back();
    }

    while (!m_albums.empty()) {
      AlbumStore::getInstance()->removeAlbum(m_albums.back());
      m_albums.pop_back();
    }

    while (!m_artists.empty()) {
      ArtistStore::getInstance()->removeArtist(m_artists.back());
      m_artists.pop_back();
    }

    Logger::printOut("cleaning after search done");

  }

  bool Search::isLoaded() {
    for (int i = 0; i < m_albums.size(); i++) {
      if (!m_albums[i]->isLoaded()) return false;
    }
    for (int i = 0; i < m_tracks.size(); i++) {
      if (!m_tracks[i]->isLoaded()) return false;
    }
    for (int i = 0; i < m_artists.size(); i++) {
      if (!m_artists[i]->isLoaded()) return false;
    }

    return true;
  }

  void Search::SP_CALLCONV cb_searchComplete(sp_search *search, void *userdata) {
    Search* searchObj = (Search*) userdata;
    searchObj->m_currentSearch = search;
    SearchResultBackgroundLoader* loader = new SearchResultBackgroundLoader(searchObj);
    loader->Create(true);
  }

} /* namespace addon_music_spotify */
