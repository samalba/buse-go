#include <IOKit/IOLib.h>
#include "IOKitBlockService_darwin.h"
#include "IOKitDiskStorageDevice_darwin.h"

OSDefineMetaClassAndStructors(buse_go_BlockService, IOService)

#define FIXED_BLOCK_SIZE (512)

#define super IOService


void buse_go_BlockService::setSizeMB(int sizeMB)
{
	this->size = ((UInt64) 1048576) * ((UInt64) sizeMB);
}

UInt64 buse_go_BlockService::getByteCount() const
{
	return this->size;
}

UInt32 buse_go_BlockService::getBlockSize() const
{
	return FIXED_BLOCK_SIZE;
}

bool buse_go_BlockService::isWritable()
{
	return true;
}

bool buse_go_BlockService::isReady()
{
	return true;
}

bool buse_go_BlockService::start(IOService *provider)
{
	IOLog("block svc starting\n");
	this->memory = NULL;
	this->buffer = NULL;
	this->nub = NULL;

	if (! super::start(provider))
		return false;
	this->memory = IOBufferMemoryDescriptor::withCapacity(this->getByteCount(), kIODirectionInOut);
	if (!this->memory)
		return false;
	this->buffer = this->memory->getBytesNoCopy();
	if (!this->buffer)
	{
		this->memory->release();
		this->memory = NULL;
		return false;
	}
	if (! this->buildDevice())
	{
		this->memory->release();
		this->memory = NULL;
		return false;
	}
	IOLog("block svc started\n");
	return true;
}

void buse_go_BlockService::free()
{
	IOLog("busego-blockservice: freeing this\n");

	if (this->memory)
	{
		this->memory->release();
		this->memory = NULL;
	}
	if (this->nub)
	{
		this->nub->release();
		this->nub = NULL;
	}
	IOLog("busego-blockservice: freeing super\n");
	super::free();
}

IOReturn buse_go_BlockService::doEjectMedia()
{
	IOLog("block svc ejecting\n");
	if (this->nub)
	{
		this->nub->detach(this);
		this->nub->stop(this);
		this->nub->release();
		this->nub = NULL;
	}

	this->detachFromParent(IORegistryEntry::getRegistryRoot(), gIOServicePlane);
	
	IOLog("block svc ejected\n");
	return kIOReturnSuccess;
}

IOByteCount buse_go_BlockService::performRead(IOMemoryDescriptor *dest, UInt64 offset, UInt64 count)
{
	return dest->writeBytes(0, (void *)(((UInt64) (this->buffer)) + offset), count);
}

IOByteCount buse_go_BlockService::performWrite(IOMemoryDescriptor *src, UInt64 offset, UInt64 count)
{
	return src->readBytes(0, (void *)(((UInt64) (this->buffer)) + offset), count);
}

bool buse_go_BlockService::buildDevice()
{
	this->nub = new buse_go_DiskStorageDevice();
	if (!this->nub)
		return false;
	if (!this->nub->init(NULL))
		return false;
	if (!this->nub->attach(this))
		return false;
	this->nub->registerService();
	return true;
}
