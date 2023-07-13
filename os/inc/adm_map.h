#ifndef _ADM_MAP_H_
#define _ADM_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ADM_UTIL_Map_t ADM_UTIL_Map_t;


/*!
 *******************************************************************************
 * Callback to compare two keys.
 *
 * \param	inKey1		First key
 * \param	inKey2		Second key
 * \param	inContext	Context passed previously to \ref DM_UTIL_Map_create
 *
 * \return	<0: \a inKey1 < \a inKey2,
 *			 0: \a inKey1 == \a inKey2,
 *			>0: \a inKey1 > \a inKey2
 *******************************************************************************
 */
typedef D32S (*ADM_UTIL_Map_Compare_CB_t) (
	const void *inKey1,  
	const void *inKey2,  
	void *inContext);

/*!
 *******************************************************************************
 * Callback to free map items.
 *
 * This function should call \ref DM_UTIL_Map_freeItem.
 *
 * \param	inKey		Key
 * \param	inValue		Value
 * \param	inContext	Context passed previously to \ref DM_UTIL_Map_create
 *
 * \return	None
 *******************************************************************************
 */
typedef void (*ADM_UTIL_Map_Free_CB_t) (void *inKey, void *inValue,
	void *inContext);

/*!
 *******************************************************************************
 * Free and nullify map items (using \ref DM_PL_freeAndNullify).
 *
 * \param	inKey		Key
 * \param	inValue		Value
 * \param	inContext	Context passed previously to \ref DM_UTIL_Map_create;
 *						the context is actually ignored
 *
 * \return	None
 *******************************************************************************
 */
void adm_map_freeItem(void *inKey, void *inValue, void *inContext);

void adm_map_setNull(ADM_UTIL_Map_t *inMap, void *inKey);

/*!
 *******************************************************************************
 * Create a map.
 *
 * \param	inCompareFunc			Callback invoked to compare two map items,
 *									a \ref DM_UTIL_Map_Compare_CB_t structure
 * \param	inFreeFunc				Callback invoked to free items,
 *									a \ref DM_UTIL_Map_Free_CB_t structure
 * \param	inFuncContext			Handle to pass to \a callback Functions
 * \param	inAllowDuplicates		Whether duplicate keys are allowed
 *
 * \return	Handle to the map on success, NULL otherwise
 *******************************************************************************
 */
extern ADM_UTIL_Map_t *adm_map_create(ADM_UTIL_Map_Compare_CB_t inCompareFunc,
	ADM_UTIL_Map_Free_CB_t inFreeFunc, void* inFuncContext,
	DBOOL inAllowDuplicates);

/*!
 *******************************************************************************
 * Destroy a map.
 *
 * \param	ioMap	Handle to the map
 *
 * \return	None
 *******************************************************************************
 */
extern void adm_map_destroy(ADM_UTIL_Map_t **ioMap);
								
/*!
 *******************************************************************************
 * Add a new key-value pair to a map.
 *
 * \param	inMap		Handle to the map
 * \param	inKey		New key
 * \param	inValue		New value
 *
 * \return	DM_ERR_OK on success, or DM_ERR_MEMORY if there is a memory
 *			problem
 *******************************************************************************
 */
extern DBOOL adm_map_add(ADM_UTIL_Map_t *inMap, void *inKey,  
	void *inValue);

/*!
 *******************************************************************************
 * Find the first item that matches a key in a map.
 *
 * \note Uses \ref DM_UTIL_Map_Compare_CB_t to compare values.
 *
 * \param	inMap		Handle to the map
 * \param	inKey		The key
 * \param	outValue	The returned value of the first matching key
 *
 * \return	TRUE if there's a matching key, FALSE otherwise
 *******************************************************************************
 */
extern DBOOL adm_map_find(ADM_UTIL_Map_t *inMap, const void *inKey,
	void **outValue);

extern DBOOL adm_map_set(ADM_UTIL_Map_t *inMap, void *inKey, void *inValue);


/*!
 *******************************************************************************
 * Iteratively get key/value pairs.
 *
 * \param	inMap		Handle to the map
 * \param	ioIter		Iterator. Should be initialized with NULL
 * \param	outKey		The returned key
 * \param	outValue	The returned value
 *
 * \return	DM_ERR_OK if it exists, DM_ERR_OUT_OF_BOUNDS if map is empty
 *******************************************************************************
 */
extern DBOOL adm_map_getNext(ADM_UTIL_Map_t *inMap, void **ioIter,
	void **outKey, void **outValue);

/*!
 *******************************************************************************
 * Remove an item from a map, according to its key.
 *
 * \note Uses \ref DM_UTIL_Map_Free_CB_t to free and nullify memory.
 *
 * \param	inMap		Handle to the map
 * \param	inKey		The key
 *
 * \return	None
 *******************************************************************************
 */
extern void adm_map_removeItem(ADM_UTIL_Map_t *inMap, void *inKey);

/*!
 *******************************************************************************
 * Remove all key/value pairs.
 *
 * \note Uses \ref DM_UTIL_Map_Free_CB_t to free and nullify memory.
 *
 * \param	inMap		Handle to the map
 *
 * \return	None
 *******************************************************************************
 */
extern void adm_map_removeAll(ADM_UTIL_Map_t *inMap);

/*!
 *******************************************************************************
 * Get the number of items in a map.
 *
 * \param	inMap	Handle to the map
 *
 * \return	Number of items in the map
 *******************************************************************************
 */
extern D32U adm_map_getSize(ADM_UTIL_Map_t *inMap);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

