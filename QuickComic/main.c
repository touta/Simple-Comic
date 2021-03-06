//==============================================================================
//
//	DO NO MODIFY THE CONTENT OF THIS FILE
//
//	This file contains the generic CFPlug-in code necessary for your generator
//	To complete your generator implement the function in GenerateThumbnailForURL/GeneratePreviewForURL.c
//
//==============================================================================






#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#include "main.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------

// Don't modify this line
#define PLUGIN_ID "D0248518-600D-47CC-9C46-EFE3A3ED6B2D"

//
// Below is the generic glue code for all plug-ins.
//
// You should not have to modify this code aside from changing
// names if you decide to change the names defined in the Info.plist
//


// -----------------------------------------------------------------------------
//	typedefs
// -----------------------------------------------------------------------------

// The layout for an instance of QuickLookGeneratorPlugIn
typedef struct __QuickLookGeneratorPluginType
{
    void        *conduitInterface;
    CFUUIDRef    factoryID;
    UInt32       refCount;
} QLGenPlug;

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//	Forward declaration for the IUnknown implementation.
//

static QLGenPlug    *AllocQuickLookGeneratorPluginType(CFUUIDRef inFactoryID);
static void          DeallocQuickLookGeneratorPluginType(QLGenPlug *thisInstance);
static HRESULT       QuickLookGeneratorQueryInterface(void *thisInstance, REFIID iid, LPVOID *ppv);
extern void         *QuickLookGeneratorPluginFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
static ULONG         QuickLookGeneratorPluginAddRef(void *thisInstance);
static ULONG         QuickLookGeneratorPluginRelease(void *thisInstance);

// -----------------------------------------------------------------------------
//	myInterfaceFtbl	definition
// -----------------------------------------------------------------------------
//	The QLGeneratorInterfaceStruct function table.
//
static QLGeneratorInterfaceStruct myInterfaceFtbl = {
    NULL,
    QuickLookGeneratorQueryInterface,
    QuickLookGeneratorPluginAddRef,
    QuickLookGeneratorPluginRelease,
    NULL,
    NULL,
    NULL,
    NULL
};


// -----------------------------------------------------------------------------
//	AllocQuickLookGeneratorPluginType
// -----------------------------------------------------------------------------
//	Utility function that allocates a new instance.
//      You can do some initial setup for the generator here if you wish
//      like allocating globals etc...
//
QLGenPlug *AllocQuickLookGeneratorPluginType(CFUUIDRef inFactoryID)
{
    QLGenPlug *theNewInstance = (QLGenPlug *)calloc(sizeof(QLGenPlug), 1);

    /* Point to the function table Malloc enough to store the stuff and copy the filler from myInterfaceFtbl over */
    theNewInstance->conduitInterface = malloc(sizeof(QLGeneratorInterfaceStruct));
    memcpy(theNewInstance->conduitInterface,&myInterfaceFtbl,sizeof(QLGeneratorInterfaceStruct));

    /*  Retain and keep an open instance refcount for each factory. */
    theNewInstance->factoryID = CFRetain(inFactoryID);
    CFPlugInAddInstanceForFactory(inFactoryID);

    /* This function returns the IUnknown interface so set the refCount to one. */
    theNewInstance->refCount = 1;
    return theNewInstance;
}

// -----------------------------------------------------------------------------
//	DeallocQuickLookGeneratorPluginType
// -----------------------------------------------------------------------------
//	Utility function that deallocates the instance when
//	the refCount goes to zero.
//      In the current implementation generator interfaces are never deallocated
//      but implement this as this might change in the future
//
void DeallocQuickLookGeneratorPluginType(QLGenPlug *thisInstance)
{
    CFUUIDRef theFactoryID;

    theFactoryID = thisInstance->factoryID;
    /* Free the conduitInterface table up */
    free(thisInstance->conduitInterface);

    /* Free the instance structure */
    free(thisInstance);
    if (theFactoryID){
        CFPlugInRemoveInstanceForFactory(theFactoryID);
        CFRelease(theFactoryID);
    }
}

// -----------------------------------------------------------------------------
//	QuickLookGeneratorQueryInterface
// -----------------------------------------------------------------------------
//	Implementation of the IUnknown QueryInterface function.
//
HRESULT QuickLookGeneratorQueryInterface(void *thisInstance, REFIID iid, LPVOID *ppv)
{
    CFUUIDRef interfaceID;

    interfaceID = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, iid);

    if (CFEqual(interfaceID, kQLGeneratorCallbacksInterfaceID)) {
        /* If the Right interface was requested, bump the ref count,
         * set the ppv parameter equal to the instance, and
         * return good status.
         */
        ((QLGeneratorInterfaceStruct *)((QLGenPlug *)thisInstance)->conduitInterface)->GenerateThumbnailForURL = GenerateThumbnailForURL;
        ((QLGeneratorInterfaceStruct *)((QLGenPlug *)thisInstance)->conduitInterface)->CancelThumbnailGeneration = CancelThumbnailGeneration;
        ((QLGeneratorInterfaceStruct *)((QLGenPlug *)thisInstance)->conduitInterface)->GeneratePreviewForURL = GeneratePreviewForURL;
        ((QLGeneratorInterfaceStruct *)((QLGenPlug *)thisInstance)->conduitInterface)->CancelPreviewGeneration = CancelPreviewGeneration;
        ((QLGeneratorInterfaceStruct *)((QLGenPlug*)thisInstance)->conduitInterface)->AddRef(thisInstance);
        *ppv = thisInstance;
        CFRelease(interfaceID);
        return S_OK;
    } else {
        /* Requested interface unknown, bail with error. */
        *ppv = NULL;
        CFRelease(interfaceID);
        return E_NOINTERFACE;
    }
}

// -----------------------------------------------------------------------------
// QuickLookGeneratorPluginAddRef
// -----------------------------------------------------------------------------
//	Implementation of reference counting for this type. Whenever an interface
//	is requested, bump the refCount for the instance. NOTE: returning the
//	refcount is a convention but is not required so don't rely on it.
//
ULONG QuickLookGeneratorPluginAddRef(void *thisInstance)
{
    return ++((QLGenPlug*)thisInstance)->refCount;
}

// -----------------------------------------------------------------------------
// QuickLookGeneratorPluginRelease
// -----------------------------------------------------------------------------
//	When an interface is released, decrement the refCount.
//	If the refCount goes to zero, deallocate the instance.
//
ULONG QuickLookGeneratorPluginRelease(void *thisInstance)
{
    ((QLGenPlug*)thisInstance)->refCount -= 1;
    if (((QLGenPlug*)thisInstance)->refCount == 0) {
        DeallocQuickLookGeneratorPluginType((QLGenPlug*)thisInstance);
        return 0;
    } else {
        return ((QLGenPlug*) thisInstance )->refCount;
    }
}

// -----------------------------------------------------------------------------
//  QuickLookGeneratorPluginFactory
// -----------------------------------------------------------------------------
void *QuickLookGeneratorPluginFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    QLGenPlug *result;
    CFUUIDRef  uuid;

    /* If correct type is being requested, allocate an
     * instance of kQLGeneratorTypeID and return the IUnknown interface.
     */
    if (CFEqual(typeID, kQLGeneratorTypeID)) {
        uuid = CFUUIDCreateFromString(kCFAllocatorDefault, CFSTR(PLUGIN_ID));
        result = AllocQuickLookGeneratorPluginType(uuid);
        CFRelease(uuid);
        return result;
    }
    /* If the requested type is incorrect, return NULL. */
    return NULL;
}
