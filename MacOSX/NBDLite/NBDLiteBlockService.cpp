#include <IOKit/IOLib.h>
#include "NBDLiteBlockService.h"
#include "NBDLiteDiskStorageDevice.h"

OSDefineMetaClassAndStructors(NBDLiteBlockService, IOService)

#define FIXED_BLOCK_SIZE (512)

#define super IOService


void NBDLiteBlockService::setSizeMB(int sizeMB)
{
	this->size = ((UInt64) 1048576) * ((UInt64) sizeMB);
}

UInt64 NBDLiteBlockService::getByteCount() const
{
	return this->size;
}

UInt32 NBDLiteBlockService::getBlockSize() const
{
	return FIXED_BLOCK_SIZE;
}

bool NBDLiteBlockService::isWritable()
{
	return true;
}

bool NBDLiteBlockService::isReady()
{
	return true;
}

bool NBDLiteBlockService::start(IOService *provider)
{
	IOLog("block svc starting\n");
	this->memory = NULL;
	this->buffer = NULL;
	this->nub = NULL;

	if (!super::start(provider))
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
	if (!this->buildDevice())
	{
		this->memory->release();
		this->memory = NULL;
		return false;
	}
	IOLog("block svc started\n");
	return true;
}

void NBDLiteBlockService::free()
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

IOReturn NBDLiteBlockService::doEjectMedia()
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

IOByteCount NBDLiteBlockService::performRead(IOMemoryDescriptor *dest, UInt64 offset, UInt64 count)
{
	return dest->writeBytes(0, (void *)(((UInt64) (this->buffer)) + offset), count);
}

IOByteCount NBDLiteBlockService::performWrite(IOMemoryDescriptor *src, UInt64 offset, UInt64 count)
{
	return src->readBytes(0, (void *)(((UInt64) (this->buffer)) + offset), count);
}

bool NBDLiteBlockService::buildDevice()
{
	this->nub = new NBDLiteDiskStorageDevice();
	if (!this->nub)
		return false;
	if (!this->nub->init(NULL))
		return false;
	if (!this->nub->attach(this))
		return false;
	this->nub->registerService();
	return true;
}
