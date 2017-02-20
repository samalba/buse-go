#ifndef __BUSE_GO__DiskStorageDevice__
#define __BUSE_GO__DiskStorageDevice__

#include <IOKit/storage/IOBlockStorageDevice.h>


class buse_go_BlockService;

class buse_go_DiskStorageDevice : public IOBlockStorageDevice
{
	OSDeclareDefaultStructors(buse_go_DiskStorageDevice)
	
private:
	buse_go_BlockService *provider;
	UInt64 blockCount;
	bool lastAskedState;

public:
	virtual bool init(OSDictionary *properties);
	virtual bool attach(IOService *provider);
	virtual void detach(IOService *provider);
	virtual IOReturn doEjectMedia();
	virtual IOReturn doFormatMedia(UInt64 byteCapacity);
	virtual UInt32 doGetFormatCapacities(UInt64 *byteCapacity, UInt32 capacitiesMaxCount) const;
	virtual IOReturn doLockUnlockMedia(bool doLock);
	virtual IOReturn doSynchronizeCache();
	virtual char * getVendorString();
	virtual char * getProductString();
	virtual char * getRevisionString();
	virtual char * getAdditionalDeviceInfoString();
	virtual IOReturn reportBlockSize(UInt64 *blockSize);
	virtual IOReturn reportEjectability(bool *isEjectable);
	virtual IOReturn reportLockability(bool *isLockable);
	virtual IOReturn reportMaxValidBlock(UInt64 *maxBlock);
	virtual IOReturn reportMediaState(bool *mediaPresent, bool *changedState);
	virtual IOReturn reportPollRequirements(bool *pollRequired, bool *pollIsExpensive);
	virtual IOReturn reportRemovability(bool *isRemovable);
	virtual IOReturn reportWriteProtection(bool *isWriteProtected);
	virtual IOReturn getWriteCacheState(bool *enabled);
	virtual IOReturn setWriteCacheState(bool enabled);
	virtual IOReturn doAsyncReadWrite(IOMemoryDescriptor *buffer, UInt64 block, UInt64 nblks, IOStorageAttributes *attributes, IOStorageCompletion *completion);
};

#endif /* !__BUSE_GO__DiskStorageDevice__ */
