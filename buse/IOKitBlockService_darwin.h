#ifndef __BUSE_GO__BlockService__
#define __BUSE_GO__BlockService__

#include <IOKit/IOService.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <sys/kpi_socket.h>

#include "IOKitDiskStorageDevice_darwin.h"


class buse_go_BlockService : public IOService
{
	OSDeclareDefaultStructors(buse_go_BlockService)

private:
	IOBufferMemoryDescriptor *memory;
	void *buffer;
	socket_t *socket;   /* 0 when uninitialized / not yet connected / error occurred and we disconnected */
	UInt64 size;
	buse_go_DiskStorageDevice *nub;
	
protected:
	bool buildDevice();
	void shutdown();

public:
	void setSizeMB(int mb);
	virtual bool start(IOService *provider);
	virtual void free();
	virtual IOByteCount performRead(IOMemoryDescriptor *dest, UInt64 offset, UInt64 count);
	virtual IOByteCount performWrite(IOMemoryDescriptor *src, UInt64 offset, UInt64 count);
	virtual IOReturn doEjectMedia();
	
	UInt64 getByteCount() const;
	UInt32 getBlockSize() const;
	bool isWritable();
	bool isReady();
};


#endif /* !__BUSE_GO__BlockService__ */
