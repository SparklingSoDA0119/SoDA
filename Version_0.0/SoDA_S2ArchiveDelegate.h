#ifndef _SODA_S2_ARCHIVE_DELEGATE_H
#define _SODA_S2_ARCHIVE_DELEGATE_H

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

////////////////////////////////////////////////////////////////////////////////

namespace SoDA {

    class S2Archive;

    class S2ArchiveDelegate {
    public:
        virtual ~S2ArchiveDelegate() {}
    public:
        virtual void archivePathAdded(S2Archive* archive) {}
        virtual void archiveItemAdded(S2Archive* archive, uint8 key) {}
        virtual void archiveItemUpdated(S2Archive* archive, uint8 key) {}
    };

}   // namespace SoDA

////////////////////////////////////////////////////////////////////////////////
#endif  // _SODA_S2_ARCHIVE_DELEGATE_H
