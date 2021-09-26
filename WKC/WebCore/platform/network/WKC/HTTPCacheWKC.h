/*
 *  Copyright (c) 2011-2013 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#ifndef HTTPCacheWKC_h
#define HTTPCacheWKC_h

#include "PlatformString.h"
#include "SharedBuffer.h"
#include "ResourceResponse.h"
#include "CurrentTime.h"
#include <wtf/HashMap.h>
#include <wtf/ListHashSet.h>
#include <wtf/Vector.h>
#include <wtf/text/StringHash.h>


namespace WebCore {

class SharedBuffer;
class KURL;
class ResourceResponse;

class HTTPCachedResource {
public:
    HTTPCachedResource();
    HTTPCachedResource(const KURL &url, const ResourceResponse &response);
    ~HTTPCachedResource();

    bool setResourceResponse(const ResourceResponse &response);
    double freshnessLifetime();
    bool isExpired();
    bool needRevalidate();
    enum {
        EHTTPEquivNone           = 0x00000000,
        EHTTPEquivNoCache        = 0x00000001,
        EHTTPEquivMustRevalidate = 0x00000002,
        EHTTPEquivMaxAge         = 0x00000004,
    };
    void update(bool noCache, bool mustRevalidate, double expires, double maxAge, int httpequivflags = 0);

    inline const String& url() const { return m_url; }
    inline const String& mimeType() const { return m_mimeType; }
    inline long long expectedContentLength() const { return m_expectedContentLength; }
    inline const String& suggestedFilename() const { return m_suggestedFilename; }
    inline const String& textEncodingName() const { return m_textEncodingName; }
    inline long long contentLength() const { return m_contentLength; }
    inline const String& fileName() const { return m_fileName; }
    inline int httpStatusCode() const { return m_httpStatusCode; }
    inline bool noCache() const { return m_noCache; }
    inline bool mustRevalidate() const { return m_mustRevalidate; }
    inline double expires() const { return m_expires; }
    inline double maxAge() const { return m_maxAge; }
    inline const String& lastModifiedHeader() const { return m_lastModifiedHeader; }
    inline const String& eTagHeader() const { return m_eTagHeader; }

    bool writeFile(const String& filename, const String& filepath);
    bool readFile(char *buf, const String& filepath);

    void calcResourceSize();
    inline int resourceSize() const { return m_resourceSize; }

    int serialize(char *buffer);
    int deserialize(char *buffer);

    void setResourceData(RefPtr<SharedBuffer> resourceData) { m_resourceData = resourceData, m_contentLength = resourceData->size(); }

    inline bool isUsed() const { return m_used; }
    inline void setUsed(bool used) { m_used = used; }

private:
    String m_url;
    String m_mimeType;
    long long m_expectedContentLength;
    String m_textEncodingName;
    String m_suggestedFilename;
    int m_httpStatusCode;
    bool m_noCache;
    bool m_mustRevalidate;
    double m_expires;
    double m_maxAge;
    double m_date;
    double m_lastModified;
    String m_lastModifiedHeader;
    String m_eTagHeader;
    String m_fileName;

    RefPtr<SharedBuffer> m_resourceData;
    long long m_contentLength;
    int m_resourceSize;
    bool m_used;

    int m_httpequivflags;
};

typedef HashMap<String, HTTPCachedResource*> HTTPCachedResourceMap;
typedef HashMap<String, HTTPCachedResource*>::iterator HTTPCachedResourceMapIterator;

class HTTPCache {
public:
    HTTPCache();
    ~HTTPCache();
    void reset();

    HTTPCachedResource* createHTTPCachedResource(KURL &url, RefPtr<SharedBuffer> resourceData, ResourceResponse &response, bool noCache, bool mustRevalidate, double expires, double maxAge);
    bool addCachedResource(HTTPCachedResource *resource);
    void updateCachedResource(HTTPCachedResource *resource, RefPtr<SharedBuffer> resourceData, ResourceResponse &response, bool noCache, bool mustRevalidate, double expires, double maxAge);
    void removeResource(HTTPCachedResource *resource);
    void removeResourceByNumber(int listNumber);
    void remove(HTTPCachedResource *resource);
    void detach(HTTPCachedResource *resource);
    void removeAll();

    void setDisabled(bool disabled);
    bool disabled() const { return m_disabled; }
    void setMaxContentEntries(int limit);
    void setMaxContentSize(long long limit);
    void setMaxTotalCacheSize(long long limit);
    void setFilePath(const char* path);

    KURL removeFragmentIdentifierIfNeeded(const KURL& originalURL);
    HTTPCachedResource* resourceForURL(const KURL& resourceURL);
    bool equalHTTPCachedResourceURL(HTTPCachedResource *resource, KURL& resourceURL);

    String makeFileName();
    long long purgeBySizeInternal(long long size);
    long long purgeBySize(long long size);
    long long purgeOldest();
    bool write(HTTPCachedResource *resource);
    bool read(HTTPCachedResource *resource, char *buf);

    void serializeFATData(char *buffer);
    bool deserializeFATData(char *buffer, int length);
    bool writeFATFile();
    bool readFATFile();

    void writeDigest(unsigned char *data, int length);
    bool verifyDigest(unsigned char *data, int length);

private:
    bool m_disabled;
    HTTPCachedResourceMap m_resources;
    Vector<HTTPCachedResource*> m_resourceList;

    int m_fileNumber;
    int m_totalResourceSize;

    String m_fatFileName;
    String m_filePath;

    int m_limitContentEntries;
    long long m_limitContentSize;
    long long m_limitTotalContentsSize;

    long long m_totalContentsSize;
};

} // namespace


#endif //HTTPCacheWKC_h
