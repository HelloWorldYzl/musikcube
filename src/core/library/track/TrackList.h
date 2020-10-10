//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2019 musikcube team
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <core/sdk/ITrackList.h>
#include <core/sdk/ITrackListEditor.h>

#include <core/library/track/Track.h>
#include <core/library/ILibrary.h>

#include <sigslot/sigslot.h>

#include <unordered_map>
#include <list>

namespace musik { namespace core {

    class TrackList :
        public musik::core::sdk::ITrackList,
        public std::enable_shared_from_this<TrackList>,
        public sigslot::has_slots<>
    {
        public:
            mutable sigslot::signal3<const TrackList*, size_t, size_t> WindowCached;

            TrackList(ILibraryPtr library);
            TrackList(TrackList* other);
            TrackList(ILibraryPtr library, const int64_t* trackIds, size_t trackIdCount);

            virtual ~TrackList();

            /* ITrackList */
            virtual size_t Count() const;
            virtual int64_t GetId(size_t index) const;
            virtual int IndexOf(int64_t id) const;
            virtual musik::core::sdk::ITrack* GetTrack(size_t index) const;
            virtual void Release() { /* not used now */ }

            /* TrackListEditor passes through to us */
            void Add(const int64_t id);
            bool Insert(int64_t id, size_t index);
            bool Swap(size_t index1, size_t index2);
            bool Move(size_t from, size_t to);
            bool Delete(size_t index);
            void Clear();
            void Shuffle();

            /* implementation specific */
            TrackPtr Get(size_t index, bool async = false) const;
            TrackPtr GetSync(size_t index) const { return this->Get(index, false); }
            TrackPtr GetAsync(size_t index) const { return this->Get(index, true); }
            void ClearCache();
            void Swap(TrackList& list);
            void CopyFrom(const TrackList& from);
            void CopyTo(TrackList& to);
            void CacheWindow(size_t from, size_t to, bool async) const;
            void SetCacheWindowSize(size_t size);
            const std::vector<int64_t> GetIds() const { return ids; };

            musik::core::sdk::ITrackList* GetSdkValue();

        private:
            struct QueryWindow {
                size_t from{ 0 };
                size_t to{ 0 };
                bool Contains(size_t i) { return to > 0 && i >= from && i <= to; }
                void Reset() { from = to = 0; }
                bool Valid() { return to > 0 && to > from; }
                void Set(size_t from, size_t to) { this->from = from; this->to = to; }
            };

            typedef std::list<int64_t> CacheList;
            typedef std::pair<TrackPtr, CacheList::iterator> CacheValue;
            typedef std::unordered_map<int64_t, CacheValue> CacheMap;

            TrackPtr GetFromCache(int64_t key) const;
            void AddToCache(int64_t key, TrackPtr value) const;
            void PruneCache() const;

            /* lru cache structures */
            mutable CacheList cacheList;
            mutable CacheMap cacheMap;
            mutable size_t cacheSize;
            mutable QueryWindow currentWindow;
            mutable QueryWindow nextWindow;

            std::vector<int64_t> ids;
            ILibraryPtr library;
    };

    class TrackListEditor : public musik::core::sdk::ITrackListEditor {
        public:
            TrackListEditor(std::shared_ptr<TrackList> trackList);
            TrackListEditor(TrackList& trackList);

            virtual ~TrackListEditor();

            virtual void Add(const int64_t id);
            virtual bool Insert(int64_t id, size_t index);
            virtual bool Swap(size_t index1, size_t index2);
            virtual bool Move(size_t from, size_t to);
            virtual bool Delete(size_t index);
            virtual void Clear();
            virtual void Shuffle();
            virtual void Release() { /* nothing yet */ }

        private:
            std::shared_ptr<TrackList> trackList;
    };
} }
