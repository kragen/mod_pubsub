/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.02.88 */
/* at Sun Dec 29 01:24:10 2002
 */
/* Compiler settings for ..\comwin32\LibKNCom.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __LibKNCom_h__
#define __LibKNCom_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IMessageEntry_FWD_DEFINED__
#define __IMessageEntry_FWD_DEFINED__
typedef interface IMessageEntry IMessageEntry;
#endif 	/* __IMessageEntry_FWD_DEFINED__ */


#ifndef __IMessage_FWD_DEFINED__
#define __IMessage_FWD_DEFINED__
typedef interface IMessage IMessage;
#endif 	/* __IMessage_FWD_DEFINED__ */


#ifndef __IComRequestStatusHandler_FWD_DEFINED__
#define __IComRequestStatusHandler_FWD_DEFINED__
typedef interface IComRequestStatusHandler IComRequestStatusHandler;
#endif 	/* __IComRequestStatusHandler_FWD_DEFINED__ */


#ifndef __IComListener_FWD_DEFINED__
#define __IComListener_FWD_DEFINED__
typedef interface IComListener IComListener;
#endif 	/* __IComListener_FWD_DEFINED__ */


#ifndef __IParameters_FWD_DEFINED__
#define __IParameters_FWD_DEFINED__
typedef interface IParameters IParameters;
#endif 	/* __IParameters_FWD_DEFINED__ */


#ifndef __IConnector_FWD_DEFINED__
#define __IConnector_FWD_DEFINED__
typedef interface IConnector IConnector;
#endif 	/* __IConnector_FWD_DEFINED__ */


#ifndef __MessageEntry_FWD_DEFINED__
#define __MessageEntry_FWD_DEFINED__

#ifdef __cplusplus
typedef class MessageEntry MessageEntry;
#else
typedef struct MessageEntry MessageEntry;
#endif /* __cplusplus */

#endif 	/* __MessageEntry_FWD_DEFINED__ */


#ifndef __Message_FWD_DEFINED__
#define __Message_FWD_DEFINED__

#ifdef __cplusplus
typedef class Message Message;
#else
typedef struct Message Message;
#endif /* __cplusplus */

#endif 	/* __Message_FWD_DEFINED__ */


#ifndef __RequestStatusHandler_FWD_DEFINED__
#define __RequestStatusHandler_FWD_DEFINED__

#ifdef __cplusplus
typedef class RequestStatusHandler RequestStatusHandler;
#else
typedef struct RequestStatusHandler RequestStatusHandler;
#endif /* __cplusplus */

#endif 	/* __RequestStatusHandler_FWD_DEFINED__ */


#ifndef __Listener_FWD_DEFINED__
#define __Listener_FWD_DEFINED__

#ifdef __cplusplus
typedef class Listener Listener;
#else
typedef struct Listener Listener;
#endif /* __cplusplus */

#endif 	/* __Listener_FWD_DEFINED__ */


#ifndef ___IConnectorEvents_FWD_DEFINED__
#define ___IConnectorEvents_FWD_DEFINED__
typedef interface _IConnectorEvents _IConnectorEvents;
#endif 	/* ___IConnectorEvents_FWD_DEFINED__ */


#ifndef __Connector_FWD_DEFINED__
#define __Connector_FWD_DEFINED__

#ifdef __cplusplus
typedef class Connector Connector;
#else
typedef struct Connector Connector;
#endif /* __cplusplus */

#endif 	/* __Connector_FWD_DEFINED__ */


#ifndef __Parameters_FWD_DEFINED__
#define __Parameters_FWD_DEFINED__

#ifdef __cplusplus
typedef class Parameters Parameters;
#else
typedef struct Parameters Parameters;
#endif /* __cplusplus */

#endif 	/* __Parameters_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IMessageEntry_INTERFACE_DEFINED__
#define __IMessageEntry_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMessageEntry
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IMessageEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("CAAB334A-BFE6-4C73-9DE3-7A052DA43B62")
    IMessageEntry : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Field( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMessageEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessageEntry __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessageEntry __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessageEntry __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessageEntry __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessageEntry __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessageEntry __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessageEntry __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Field )( 
            IMessageEntry __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Value )( 
            IMessageEntry __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        END_INTERFACE
    } IMessageEntryVtbl;

    interface IMessageEntry
    {
        CONST_VTBL struct IMessageEntryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessageEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessageEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessageEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessageEntry_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessageEntry_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessageEntry_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessageEntry_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessageEntry_get_Field(This,pVal)	\
    (This)->lpVtbl -> get_Field(This,pVal)

#define IMessageEntry_get_Value(This,pVal)	\
    (This)->lpVtbl -> get_Value(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMessageEntry_get_Field_Proxy( 
    IMessageEntry __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IMessageEntry_get_Field_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMessageEntry_get_Value_Proxy( 
    IMessageEntry __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IMessageEntry_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMessageEntry_INTERFACE_DEFINED__ */


#ifndef __IMessage_INTERFACE_DEFINED__
#define __IMessage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IMessage
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("9FED2E8D-A992-4DA9-872C-110C55B1AF92")
    IMessage : public IDispatch
    {
    public:
        virtual /* [restricted][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Set( 
            /* [in] */ BSTR field,
            /* [in] */ BSTR value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Get( 
            /* [in] */ BSTR field,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ BSTR field) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsEmpty( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IMessage __RPC_FAR *msg) = 0;
        
        virtual /* [hidden][restricted][helpstring][id] */ HRESULT STDMETHODCALLTYPE _GetImpl( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMessage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMessage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMessage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMessage __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMessage __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMessage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMessage __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [restricted][helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMessage __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Set )( 
            IMessage __RPC_FAR * This,
            /* [in] */ BSTR field,
            /* [in] */ BSTR value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Get )( 
            IMessage __RPC_FAR * This,
            /* [in] */ BSTR field,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            IMessage __RPC_FAR * This,
            /* [in] */ BSTR field);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Empty )( 
            IMessage __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEmpty )( 
            IMessage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Copy )( 
            IMessage __RPC_FAR * This,
            /* [in] */ IMessage __RPC_FAR *msg);
        
        /* [hidden][restricted][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *_GetImpl )( 
            IMessage __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        END_INTERFACE
    } IMessageVtbl;

    interface IMessage
    {
        CONST_VTBL struct IMessageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessage_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IMessage_Set(This,field,value)	\
    (This)->lpVtbl -> Set(This,field,value)

#define IMessage_Get(This,field,pVal)	\
    (This)->lpVtbl -> Get(This,field,pVal)

#define IMessage_Remove(This,field)	\
    (This)->lpVtbl -> Remove(This,field)

#define IMessage_Empty(This)	\
    (This)->lpVtbl -> Empty(This)

#define IMessage_IsEmpty(This,pVal)	\
    (This)->lpVtbl -> IsEmpty(This,pVal)

#define IMessage_Copy(This,msg)	\
    (This)->lpVtbl -> Copy(This,msg)

#define IMessage__GetImpl(This,pVal)	\
    (This)->lpVtbl -> _GetImpl(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMessage_get__NewEnum_Proxy( 
    IMessage __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IMessage_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage_Set_Proxy( 
    IMessage __RPC_FAR * This,
    /* [in] */ BSTR field,
    /* [in] */ BSTR value);


void __RPC_STUB IMessage_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage_Get_Proxy( 
    IMessage __RPC_FAR * This,
    /* [in] */ BSTR field,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IMessage_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage_Remove_Proxy( 
    IMessage __RPC_FAR * This,
    /* [in] */ BSTR field);


void __RPC_STUB IMessage_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage_Empty_Proxy( 
    IMessage __RPC_FAR * This);


void __RPC_STUB IMessage_Empty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage_IsEmpty_Proxy( 
    IMessage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IMessage_IsEmpty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage_Copy_Proxy( 
    IMessage __RPC_FAR * This,
    /* [in] */ IMessage __RPC_FAR *msg);


void __RPC_STUB IMessage_Copy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [hidden][restricted][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMessage__GetImpl_Proxy( 
    IMessage __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IMessage__GetImpl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMessage_INTERFACE_DEFINED__ */


#ifndef __IComRequestStatusHandler_INTERFACE_DEFINED__
#define __IComRequestStatusHandler_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IComRequestStatusHandler
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IComRequestStatusHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("72AE2756-8E4C-4eca-AE07-16E49439B177")
    IComRequestStatusHandler : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnStatus( 
            /* [in] */ IMessage __RPC_FAR *msg) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComRequestStatusHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComRequestStatusHandler __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComRequestStatusHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComRequestStatusHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IComRequestStatusHandler __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IComRequestStatusHandler __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IComRequestStatusHandler __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IComRequestStatusHandler __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnStatus )( 
            IComRequestStatusHandler __RPC_FAR * This,
            /* [in] */ IMessage __RPC_FAR *msg);
        
        END_INTERFACE
    } IComRequestStatusHandlerVtbl;

    interface IComRequestStatusHandler
    {
        CONST_VTBL struct IComRequestStatusHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComRequestStatusHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComRequestStatusHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComRequestStatusHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComRequestStatusHandler_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComRequestStatusHandler_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComRequestStatusHandler_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComRequestStatusHandler_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComRequestStatusHandler_OnStatus(This,msg)	\
    (This)->lpVtbl -> OnStatus(This,msg)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IComRequestStatusHandler_OnStatus_Proxy( 
    IComRequestStatusHandler __RPC_FAR * This,
    /* [in] */ IMessage __RPC_FAR *msg);


void __RPC_STUB IComRequestStatusHandler_OnStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComRequestStatusHandler_INTERFACE_DEFINED__ */


#ifndef __IComListener_INTERFACE_DEFINED__
#define __IComListener_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IComListener
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IComListener;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("42A593D0-96CF-4feb-BB54-C6AA44DE1C30")
    IComListener : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OnUpdate( 
            /* [in] */ IMessage __RPC_FAR *msg) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComListenerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComListener __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComListener __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComListener __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IComListener __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IComListener __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IComListener __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IComListener __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnUpdate )( 
            IComListener __RPC_FAR * This,
            /* [in] */ IMessage __RPC_FAR *msg);
        
        END_INTERFACE
    } IComListenerVtbl;

    interface IComListener
    {
        CONST_VTBL struct IComListenerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComListener_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComListener_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComListener_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComListener_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComListener_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComListener_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComListener_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComListener_OnUpdate(This,msg)	\
    (This)->lpVtbl -> OnUpdate(This,msg)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IComListener_OnUpdate_Proxy( 
    IComListener __RPC_FAR * This,
    /* [in] */ IMessage __RPC_FAR *msg);


void __RPC_STUB IComListener_OnUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComListener_INTERFACE_DEFINED__ */


#ifndef __IParameters_INTERFACE_DEFINED__
#define __IParameters_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IParameters
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IParameters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0B3E3DD2-AF93-4FA7-8243-6D8B10BDD8F1")
    IParameters : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ServerUrl( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ServerUrl( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Username( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Username( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Password( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Password( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UseProxy( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UseProxy( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProxyServer( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ProxyServer( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProxyUsername( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ProxyUsername( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProxyPassword( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ProxyPassword( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProxyExceptionList( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ProxyExceptionList( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CustomHeader( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CustomHeader( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ShowUI( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ShowUI( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IParametersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IParameters __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IParameters __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IParameters __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IParameters __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IParameters __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IParameters __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IParameters __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ServerUrl )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ServerUrl )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Username )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Username )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Password )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Password )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UseProxy )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_UseProxy )( 
            IParameters __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProxyServer )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProxyServer )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProxyUsername )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProxyUsername )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProxyPassword )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProxyPassword )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProxyExceptionList )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProxyExceptionList )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CustomHeader )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CustomHeader )( 
            IParameters __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ShowUI )( 
            IParameters __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ShowUI )( 
            IParameters __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IParametersVtbl;

    interface IParameters
    {
        CONST_VTBL struct IParametersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IParameters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IParameters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IParameters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IParameters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IParameters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IParameters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IParameters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IParameters_get_ServerUrl(This,pVal)	\
    (This)->lpVtbl -> get_ServerUrl(This,pVal)

#define IParameters_put_ServerUrl(This,newVal)	\
    (This)->lpVtbl -> put_ServerUrl(This,newVal)

#define IParameters_get_Username(This,pVal)	\
    (This)->lpVtbl -> get_Username(This,pVal)

#define IParameters_put_Username(This,newVal)	\
    (This)->lpVtbl -> put_Username(This,newVal)

#define IParameters_get_Password(This,pVal)	\
    (This)->lpVtbl -> get_Password(This,pVal)

#define IParameters_put_Password(This,newVal)	\
    (This)->lpVtbl -> put_Password(This,newVal)

#define IParameters_get_UseProxy(This,pVal)	\
    (This)->lpVtbl -> get_UseProxy(This,pVal)

#define IParameters_put_UseProxy(This,newVal)	\
    (This)->lpVtbl -> put_UseProxy(This,newVal)

#define IParameters_get_ProxyServer(This,pVal)	\
    (This)->lpVtbl -> get_ProxyServer(This,pVal)

#define IParameters_put_ProxyServer(This,newVal)	\
    (This)->lpVtbl -> put_ProxyServer(This,newVal)

#define IParameters_get_ProxyUsername(This,pVal)	\
    (This)->lpVtbl -> get_ProxyUsername(This,pVal)

#define IParameters_put_ProxyUsername(This,newVal)	\
    (This)->lpVtbl -> put_ProxyUsername(This,newVal)

#define IParameters_get_ProxyPassword(This,pVal)	\
    (This)->lpVtbl -> get_ProxyPassword(This,pVal)

#define IParameters_put_ProxyPassword(This,newVal)	\
    (This)->lpVtbl -> put_ProxyPassword(This,newVal)

#define IParameters_get_ProxyExceptionList(This,pVal)	\
    (This)->lpVtbl -> get_ProxyExceptionList(This,pVal)

#define IParameters_put_ProxyExceptionList(This,newVal)	\
    (This)->lpVtbl -> put_ProxyExceptionList(This,newVal)

#define IParameters_get_CustomHeader(This,pVal)	\
    (This)->lpVtbl -> get_CustomHeader(This,pVal)

#define IParameters_put_CustomHeader(This,newVal)	\
    (This)->lpVtbl -> put_CustomHeader(This,newVal)

#define IParameters_get_ShowUI(This,pVal)	\
    (This)->lpVtbl -> get_ShowUI(This,pVal)

#define IParameters_put_ShowUI(This,newVal)	\
    (This)->lpVtbl -> put_ShowUI(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_ServerUrl_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_ServerUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_ServerUrl_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_ServerUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_Username_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_Username_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_Username_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_Username_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_Password_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_Password_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_UseProxy_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_UseProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_UseProxy_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IParameters_put_UseProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_ProxyServer_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_ProxyServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_ProxyServer_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_ProxyServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_ProxyUsername_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_ProxyUsername_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_ProxyUsername_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_ProxyUsername_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_ProxyPassword_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_ProxyPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_ProxyPassword_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_ProxyPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_ProxyExceptionList_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_ProxyExceptionList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_ProxyExceptionList_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_ProxyExceptionList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_CustomHeader_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_CustomHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_CustomHeader_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IParameters_put_CustomHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IParameters_get_ShowUI_Proxy( 
    IParameters __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IParameters_get_ShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IParameters_put_ShowUI_Proxy( 
    IParameters __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IParameters_put_ShowUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IParameters_INTERFACE_DEFINED__ */


#ifndef __IConnector_INTERFACE_DEFINED__
#define __IConnector_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IConnector
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][dual][uuid][object] */ 



EXTERN_C const IID IID_IConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("AA24822F-7377-44B2-8FDC-B0569C2EBDBF")
    IConnector : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsConnected( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ IParameters __RPC_FAR *p,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetParameters( 
            /* [retval][out] */ IParameters __RPC_FAR *__RPC_FAR *p) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Close( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnsureConnected( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Publish( 
            /* [in] */ IMessage __RPC_FAR *m,
            /* [in] */ IComRequestStatusHandler __RPC_FAR *sh) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IConnector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IConnector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IConnector __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IConnector __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IConnector __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IConnector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IConnector __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsConnected )( 
            IConnector __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IConnector __RPC_FAR * This,
            /* [in] */ IParameters __RPC_FAR *p,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParameters )( 
            IConnector __RPC_FAR * This,
            /* [retval][out] */ IParameters __RPC_FAR *__RPC_FAR *p);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IConnector __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnsureConnected )( 
            IConnector __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Publish )( 
            IConnector __RPC_FAR * This,
            /* [in] */ IMessage __RPC_FAR *m,
            /* [in] */ IComRequestStatusHandler __RPC_FAR *sh);
        
        END_INTERFACE
    } IConnectorVtbl;

    interface IConnector
    {
        CONST_VTBL struct IConnectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConnector_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IConnector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IConnector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IConnector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IConnector_IsConnected(This,pVal)	\
    (This)->lpVtbl -> IsConnected(This,pVal)

#define IConnector_Open(This,p,pVal)	\
    (This)->lpVtbl -> Open(This,p,pVal)

#define IConnector_GetParameters(This,p)	\
    (This)->lpVtbl -> GetParameters(This,p)

#define IConnector_Close(This,pVal)	\
    (This)->lpVtbl -> Close(This,pVal)

#define IConnector_EnsureConnected(This,pVal)	\
    (This)->lpVtbl -> EnsureConnected(This,pVal)

#define IConnector_Publish(This,m,sh)	\
    (This)->lpVtbl -> Publish(This,m,sh)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IConnector_IsConnected_Proxy( 
    IConnector __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IConnector_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IConnector_Open_Proxy( 
    IConnector __RPC_FAR * This,
    /* [in] */ IParameters __RPC_FAR *p,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IConnector_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IConnector_GetParameters_Proxy( 
    IConnector __RPC_FAR * This,
    /* [retval][out] */ IParameters __RPC_FAR *__RPC_FAR *p);


void __RPC_STUB IConnector_GetParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IConnector_Close_Proxy( 
    IConnector __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IConnector_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IConnector_EnsureConnected_Proxy( 
    IConnector __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IConnector_EnsureConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IConnector_Publish_Proxy( 
    IConnector __RPC_FAR * This,
    /* [in] */ IMessage __RPC_FAR *m,
    /* [in] */ IComRequestStatusHandler __RPC_FAR *sh);


void __RPC_STUB IConnector_Publish_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IConnector_INTERFACE_DEFINED__ */



#ifndef __LIBKNCOMLib_LIBRARY_DEFINED__
#define __LIBKNCOMLib_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: LIBKNCOMLib
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_LIBKNCOMLib;

EXTERN_C const CLSID CLSID_MessageEntry;

#ifdef __cplusplus

class DECLSPEC_UUID("F146EBAB-B76E-4CB7-BDA4-9BE413D03C21")
MessageEntry;
#endif

EXTERN_C const CLSID CLSID_Message;

#ifdef __cplusplus

class DECLSPEC_UUID("B32E3222-730B-493C-BD62-052D45383B38")
Message;
#endif

EXTERN_C const CLSID CLSID_RequestStatusHandler;

#ifdef __cplusplus

class DECLSPEC_UUID("84458F40-2A17-491b-A6BF-1C8C96E853DC")
RequestStatusHandler;
#endif

EXTERN_C const CLSID CLSID_Listener;

#ifdef __cplusplus

class DECLSPEC_UUID("FC37C6D8-2463-4b0f-8397-924FA5314BA2")
Listener;
#endif

#ifndef ___IConnectorEvents_DISPINTERFACE_DEFINED__
#define ___IConnectorEvents_DISPINTERFACE_DEFINED__

/****************************************
 * Generated header for dispinterface: _IConnectorEvents
 * at Sun Dec 29 01:24:10 2002
 * using MIDL 3.02.88
 ****************************************/
/* [helpstring][uuid] */ 



EXTERN_C const IID DIID__IConnectorEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    interface DECLSPEC_UUID("F91E756A-C698-42ba-BDDC-D93971125AA9")
    _IConnectorEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IConnectorEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IConnectorEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IConnectorEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IConnectorEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IConnectorEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IConnectorEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IConnectorEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IConnectorEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IConnectorEventsVtbl;

    interface _IConnectorEvents
    {
        CONST_VTBL struct _IConnectorEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IConnectorEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IConnectorEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IConnectorEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IConnectorEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IConnectorEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IConnectorEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IConnectorEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IConnectorEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_Connector;

#ifdef __cplusplus

class DECLSPEC_UUID("AF9469CA-A307-4BF9-BD3E-2F65E6FC87A9")
Connector;
#endif

EXTERN_C const CLSID CLSID_Parameters;

#ifdef __cplusplus

class DECLSPEC_UUID("516BB624-7FDA-4712-A8CD-474EB1844661")
Parameters;
#endif
#endif /* __LIBKNCOMLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
