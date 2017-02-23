#ifndef __NBDLiteBlockService__
#define __NBDLiteBlockService__

#include <IOKit/IOService.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

#include "NBDLiteDiskStorageDevice.h"


class NBDLiteBlockService : public IOService
{
	OSDeclareDefaultStructors(NBDLiteBlockService)

private:
	IOBufferMemoryDescriptor *memory;
	void *buffer;
	UInt64 size;
	NBDLiteDiskStorageDevice *nub;
	
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


#endif /* !__NBDLiteBlockService__ */
