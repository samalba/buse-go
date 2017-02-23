#include <IOKit/storage/IOBlockStorageDevice.h>
#include <sys/types.h>
#include <miscfs/devfs/devfs.h>
#include <sys/buf.h>
#include <sys/fcntl.h>
#include <sys/ioccom.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/systm.h>
#include <IOKit/assert.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/storage/IOMedia.h>
#include "NBDLiteDiskStorageDevice.h"
#include "NBDLiteBlockService.h"

#define super IOBlockStorageDevice


OSDefineMetaClassAndStructors(NBDLiteDiskStorageDevice, super)


bool NBDLiteDiskStorageDevice::init(OSDictionary *properties)
{
	if (!super::init(properties))
		return false;
	this->setProperty(kIOBSDNameKey, "NBDLite");
	this->setProperty(kIOBSDMajorKey, 92);
	return true;
}

bool NBDLiteDiskStorageDevice::attach(IOService *provider)
{
	if (!super::attach(provider))
		return false;
	this->provider = OSDynamicCast(NBDLiteBlockService, provider);
	if (!this->provider)
		return false;
	if (this->provider->getByteCount() % this->provider->getBlockSize())
		return false;
	this->blockCount = this->provider->getByteCount() / this->provider->getBlockSize();
	this->lastAskedState = this->provider->isReady();
	return true;
}

void NBDLiteDiskStorageDevice::detach(IOService *provider)
{
	if (provider == this->provider)
		this->provider = NULL;
	super::detach(provider);
}

IOReturn NBDLiteDiskStorageDevice::doEjectMedia()
{
	this->provider->doEjectMedia();
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::doFormatMedia(UInt64 byteCapacity)
{
	return kIOReturnUnsupported;
}

UInt32 NBDLiteDiskStorageDevice::doGetFormatCapacities(UInt64 *byteCapacities, UInt32 capacitiesMaxCount) const
{
	if (!byteCapacities)
		return 1;
	if (capacitiesMaxCount < 1)
		return 0;
	byteCapacities[0] = this->provider->getByteCount();
	return 1;
}

IOReturn NBDLiteDiskStorageDevice::doLockUnlockMedia(bool doLock)
{
	if (doLock)
		return kIOReturnUnsupported;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::doSynchronizeCache()
{
	return kIOReturnSuccess;
}

char *NBDLiteDiskStorageDevice::getVendorString()
{
	return (char *) "(networked)";
}

char *NBDLiteDiskStorageDevice::getProductString()
{
	return (char *) "Buse-go Disk";
}

char *NBDLiteDiskStorageDevice::getRevisionString()
{
	return (char *) "1";
}

char *NBDLiteDiskStorageDevice::getAdditionalDeviceInfoString()
{
	return (char *) "buse-go device size=%lld bytes";
}

IOReturn NBDLiteDiskStorageDevice::reportBlockSize(UInt64 *blockSize)
{
	*blockSize = (UInt64) this->provider->getBlockSize();
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportEjectability(bool *isEjectable)
{
	*isEjectable = true;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportLockability(bool *isLockable)
{
	*isLockable = false;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportMaxValidBlock(UInt64 *maxBlock)
{
	*maxBlock = this->blockCount - 1;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportMediaState(bool *mediaPresent, bool *changedState)
{
	const bool ready = (this->provider && this->provider->isReady());
	*mediaPresent = ready;
	*changedState = (this->lastAskedState != ready);
	this->lastAskedState = ready;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportPollRequirements(bool *pollRequired, bool *pollIsExpensive)
{
	*pollRequired = true;
	*pollIsExpensive = false;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportRemovability(bool *isRemovable)
{
	*isRemovable = true;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::reportWriteProtection(bool *isWriteProtected)
{
	*isWriteProtected = ! (this->provider && this->provider->isWritable());
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::getWriteCacheState(bool *enabled)
{
	*enabled = false;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::setWriteCacheState(bool enabled)
{
	if (enabled)
		return kIOReturnUnsupported;
	return kIOReturnSuccess;
}

IOReturn NBDLiteDiskStorageDevice::doAsyncReadWrite(IOMemoryDescriptor *buffer, UInt64 block, UInt64 nblks, IOStorageAttributes *attributes, IOStorageCompletion *completion)
{
	IOByteCount actualCount = 0;
	
	NBDLiteBlockService *provider = this->provider;
	if (!(provider && this->provider->isReady()) )
		return kIOReturnNotAttached;
	if ((block + nblks) > (this->blockCount) )
		return kIOReturnBadArgument;
	const UInt32 blockSize = provider->getBlockSize();
	if (buffer->getDirection() == kIODirectionIn)
	{
		actualCount = provider->performRead(
			buffer,
			block * blockSize,
			nblks * blockSize);
	}
	else if (buffer->getDirection() == kIODirectionOut)
	{
		if (!provider->isWritable())
			return kIOReturnNotWritable;
		actualCount = provider->performWrite(
			buffer,
			block * blockSize,
			nblks * blockSize);
	}
	else
		return kIOReturnBadArgument;
	(completion->action)(completion->target, completion->parameter, kIOReturnSuccess, actualCount);
	return kIOReturnSuccess;
}
