#define DSCLIENT_RETRY_OPERATION_WRITE(aQuery, iOperationType) 
    do { 
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_OPERATION_WRITE");
      boost::shared_ptr<mongo::DBClientCursor> dbcc;
      if (!option.retry()) { 
	try { 
	  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
	  dbcc = _conn.getInternals()->get()->query( 
						    _dbname+".$cmd", 
						    aQuery, 
						    1, 
						    0,
						    NULL,
						    0,
						    0
						     );
	  //(aQuery) 
	}
	DSCLIENT_TRACE_DEBUG("DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW");
	 catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
	//(ConnectionException, mongo::SocketException, ConnectionFails, "Probably a connection Error during reconnection")
	  catch(mongo::UserException& iEx) {  
	    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
	    }
	    else {
	      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
	    }
	  }
	//(iOperationType, iExceptionType, iErrorCode)
	 catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
	//(iExceptionType, mongo::DBException, iErrorCode, "Error occurred while executing " #iOperationType ".")
	  //(iOperationType, WriteException, WriteError)
	  if (!dbcc.get()) {
	    DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
	  }else if (!dbcc->more()) {
	    DSCLIENT_THROW(ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
	  }
	  else {
	    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_WRITE_ERROR_OR_RETURN");
	    mongo::BSONObj writeResult = dbcc->nextSafe().copy();
	    if (writeResult.hasElement("writeErrors") || writeResult.hasElement("writeConcernError") || writeResult.hasElement("errmsg")) {
	      DSCLIENT_THROW(WriteException, WriteError, "Error while performing " #iOperationType ": " + writeResult.toString());
	    }else {
	      return writeResult;
	    }
	  }
	//(iOperationType) 
      }
      else {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP");
	Retry retry = option.retry().get();
	boost::shared_ptr<RetryObject> pRetryObject;
	Retry_pattern retry_pattern = retry._retry_pattern;
	switch(retry_pattern) {
	case SimpleRetry:
	  pRetryObject.reset(new SimpleRetryObject(retry._retryIntervalMS));
	  break;
	case LinearRetry:
	  pRetryObject.reset(new LinearRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
	  break;
	case ExponentialRetry:
	  pRetryObject.reset(new ExponentialRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
	  break;
	}
	bool hasUpdatedRetryCounterForCurrentTry;

	do {
	  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_WRITE");
	  try {
	    hasUpdatedRetryCounterForCurrentTry = false;
	    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
	    dbcc = _conn.getInternals()->get()->query( 
						      _dbname+".$cmd", 
						      aQuery, 
						      1, 
						      0,
						      NULL,
						      0,
						      0
						       );
	    //(aQuery) 
	  }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
	    pRetryObject->updateCounter();  /*update retry counter*/
	    if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	      pRetryObject->waitUntilNextTry();
	      hasUpdatedRetryCounterForCurrentTry = true;
	    }else {
	      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
	    }
	    _conn.getInternals()->failover(); 
	  }
	  catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
	    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	      pRetryObject->updateCounter();  /*update retry counter*/
	      if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
		pRetryObject->waitUntilNextTry();
		hasUpdatedRetryCounterForCurrentTry = true;
	      }else {
		__DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
	      }
	      _conn.getInternals()->failover(); 
	    }else {
	      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
	    }
	  }
	   catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
	  //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
	    //(iOperationType, WriteException, WriteError) 
	    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
	  if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
	    if (!hasUpdatedRetryCounterForCurrentTry)  { 
	      pRetryObject->updateCounter(); 
	      if (pRetryObject->shouldRetry()) { 
		pRetryObject->waitUntilNextTry();
		hasUpdatedRetryCounterForCurrentTry = true;
	      }else { 
		DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
	      }
	    }
	    _conn.getInternals()->failover(); 
	  }
	  (iOperationType)
	  else if (!dbcc->more()) {
	    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
	    if (!hasUpdatedRetryCounterForCurrentTry)  {
	      pRetryObject->updateCounter(); 
	      if (pRetryObject->shouldRetry()) { 
		pRetryObject->waitUntilNextTry();
		hasUpdatedRetryCounterForCurrentTry = true;
	      }else {
		DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
	      }
	    }
	    _conn.getInternals()->failover(); 
	  }
	  //(iOperationType)
	  else {
	    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_WRITE_ERROR_OR_RETURN");
	    mongo::BSONObj writeResult = dbcc->nextSafe().copy();
	    if (writeResult.hasElement("writeErrors") || writeResult.hasElement("writeConcernError") || writeResult.hasElement("errmsg")) {
	      DSCLIENT_THROW(WriteException, WriteError, "Error while performing " #iOperationType ": " + writeResult.toString());
	    }else {
	      return writeResult;
	    }
	  }
	  //(iOperationType) 
	}while(pRetryObject->shouldRetry());
	//(aQuery, iOperationType )
      }
    }while(0);

#define DSCLIENT_RETRY_OPERATION_FIND(aQuery, iOperationType) 
do { 
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_OPERATION_FIND");
  boost::shared_ptr<mongo::DBClientCursor> dbcc;
  if (!option.retry()) {
    try {
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_FIND_QUERY");
      dbcc = _conn.getInternals()->get()->query(
						_dbname+"."+_collname,
						aQuery,
						option.limit().get_value_or(0),
						option.skip().get_value_or(0),
						option.projection().get_ptr(),
						(mongo::QueryOptions) queryOptions,
						option.batchSize().get_value_or(0)
						);
      //(aQuery) 
    }DSCLIENT_TRACE_DEBUG("DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW");
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(ConnectionException, mongo::SocketException, ConnectionFails, "Probably a connection Error during reconnection")
       catch(mongo::UserException& iEx) {  
	 if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
	 }
	 else {
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
	 }
       }
    //(iOperationType, iExceptionType, iErrorCode)
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(iExceptionType, mongo::DBException, iErrorCode, "Error occurred while executing " #iOperationType ".")
      //(iOperationType, QueryException, QueryError)
      if (!dbcc.get()) {
	DSCLIENT_TRACE_DEBUG("DBCC IS NULL");
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }else {
	DSCLIENT_TRACE_DEBUG("DBCC IS NOT NULL");
	std::auto_ptr<Cursor> pcurs(new Cursor(dbcc));
	return pcurs;
      }
  }
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP");
    Retry retry = option.retry().get();
    boost::shared_ptr<RetryObject> pRetryObject;
    Retry_pattern retry_pattern = retry._retry_pattern;
    switch(retry_pattern) {
    case SimpleRetry:
      pRetryObject.reset(new SimpleRetryObject(retry._retryIntervalMS));
      break;
    case LinearRetry:
      pRetryObject.reset(new LinearRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    case ExponentialRetry:
      pRetryObject.reset(new ExponentialRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    }
    bool hasUpdatedRetryCounterForCurrentTry;

    DSCLIENT_TRACE_DEBUG("BETWEEN SETUP AND OPERATION FIND");
    do {
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_FIND");
      try {
	hasUpdatedRetryCounterForCurrentTry = false;
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_FIND_QUERY");
	dbcc = _conn.getInternals()->get()->query(
						  _dbname+"."+_collname,
						  aQuery,
						  option.limit().get_value_or(0),
						  option.skip().get_value_or(0),
						  option.projection().get_ptr(),
						  (mongo::QueryOptions) queryOptions,
						  option.batchSize().get_value_or(0)
						  );
	//(aQuery) 
      }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
	pRetryObject->updateCounter();  /*update retry counter*/
	if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	  pRetryObject->waitUntilNextTry();
	  hasUpdatedRetryCounterForCurrentTry = true;
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
	}
	_conn.getInternals()->failover(); 
      }
      catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
	if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	  pRetryObject->updateCounter();  /*update retry counter*/
	  if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else {
	    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
	  }
	  _conn.getInternals()->failover(); 
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
	}
      }
       catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
      //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
	//(iOperationType, QueryException, QueryError) 
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
      if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
	if (!hasUpdatedRetryCounterForCurrentTry)  { 
	  pRetryObject->updateCounter(); 
	  if (pRetryObject->shouldRetry()) { 
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else { 
	    DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
	  }
	}
	_conn.getInternals()->failover(); 
      }
      //(iOperationType)
      else {
	DSCLIENT_TRACE_INFO("xxx");
	std::auto_ptr<Cursor> pcurs(new Cursor(dbcc));
	return pcurs;
      }
    }while(pRetryObject->shouldRetry());
    //(aQuery, iOperationType )
  }
 }while(0);

#define DSCLIENT_RETRY_OPERATION_AGGREGATE(aQuery, iOperationType) 
do { 
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_OPERATION_AGGREGATE");
  boost::shared_ptr<mongo::DBClientCursor> dbcc;
  if (!option.retry()) {
    try {
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_AGGREGATE_QUERY");
      dbcc =_conn.getInternals()->get()->query(
					       _dbname+".$cmd",
					       aQuery,
					       1,
					       0,
					       NULL,
					       (mongo::QueryOptions) queryOptions,
					       0
					       );
      //(aQuery) 
    }DSCLIENT_TRACE_DEBUG("DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW");
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(ConnectionException, mongo::SocketException, ConnectionFails, "Probably a connection Error during reconnection")
       catch(mongo::UserException& iEx) {  
	 if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
	 }
	 else {
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
	 }
       }
    //(iOperationType, iExceptionType, iErrorCode)
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(iExceptionType, mongo::DBException, iErrorCode, "Error occurred while executing " #iOperationType ".")
      //(iOperationType, QueryException, QueryError)
      if (!dbcc.get()) {
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }else {
	std::auto_ptr<Cursor> pcurs(new Cursor(dbcc));
	return pcurs;
      }
  }
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP");
    Retry retry = option.retry().get();
    boost::shared_ptr<RetryObject> pRetryObject;
    Retry_pattern retry_pattern = retry._retry_pattern;
    switch(retry_pattern) {
    case SimpleRetry:
      pRetryObject.reset(new SimpleRetryObject(retry._retryIntervalMS));
      break;
    case LinearRetry:
      pRetryObject.reset(new LinearRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    case ExponentialRetry:
      pRetryObject.reset(new ExponentialRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    }
    bool hasUpdatedRetryCounterForCurrentTry;

    do {
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_AGGREGATE");
      try {
	hasUpdatedRetryCounterForCurrentTry = false;
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_AGGREGATE_QUERY");
	dbcc =_conn.getInternals()->get()->query(
						 _dbname+".$cmd",
						 aQuery,
						 1,
						 0,
						 NULL,
						 (mongo::QueryOptions) queryOptions,
						 0
						 );
	//(aQuery) 
      }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
	pRetryObject->updateCounter();  /*update retry counter*/
	if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	  pRetryObject->waitUntilNextTry();
	  hasUpdatedRetryCounterForCurrentTry = true;
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
	}
	_conn.getInternals()->failover(); 
      }
      catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
	if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	  pRetryObject->updateCounter();  /*update retry counter*/
	  if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else {
	    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
	  }
	  _conn.getInternals()->failover(); 
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
	}
      }
       catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
      //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
	(iOperationType, QueryException, QueryError) 
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
      if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
	if (!hasUpdatedRetryCounterForCurrentTry)  { 
	  pRetryObject->updateCounter(); 
	  if (pRetryObject->shouldRetry()) { 
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else { 
	    DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
	  }
	}
	_conn.getInternals()->failover(); 
      }
      //(iOperationType)
      else {
	std::auto_ptr<Cursor> pcurs(new Cursor(dbcc));
	return pcurs;
      }
    }while(pRetryObject->shouldRetry());
    //(aQuery, iOperationType )
  }
 }while(0);

#define DSCLIENT_RETRY_OPERATION_COUNT(aQuery, iOperationType) 
do { 
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_OPERATION_COUNT");
  std::auto_ptr<mongo::DBClientCursor> dbcc;
  if (!option.retry()) { 
    try { 
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
      dbcc = _conn.getInternals()->get()->query( 
						_dbname+".$cmd", 
						aQuery, 
						1, 
						0,
						NULL,
						0,
						0
						 );
      //(aQuery) 
    }DSCLIENT_TRACE_DEBUG("DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW");
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(ConnectionException, mongo::SocketException, ConnectionFails, "Probably a connection Error during reconnection")
       catch(mongo::UserException& iEx) {  
	 if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
	 }
	 else {
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
	 }
       }
    //(iOperationType, iExceptionType, iErrorCode)
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(iExceptionType, mongo::DBException, iErrorCode, "Error occurred while executing " #iOperationType ".")
      //(iOperationType, QueryException, QueryError)
      if (!dbcc.get()) {
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }else if (!dbcc->more()) {
	DSCLIENT_THROW(ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
      }
      else {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_COUNT_ERROR_OR_RETURN");
	mongo::BSONObj resultObj = dbcc->nextSafe().copy();
	if (!resultObj.hasElement("n")) {
	  DSCLIENT_THROW(QueryException, QueryError, "No nested 'n' element: " + resultObj.toString());
	}else {
	  return static_cast<uint64_t>(resultObj["n"].Number());;
	}
      }
    //(iOperationType) 
  }
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP");
    Retry retry = option.retry().get();
    boost::shared_ptr<RetryObject> pRetryObject;
    Retry_pattern retry_pattern = retry._retry_pattern;
    switch(retry_pattern) {
    case SimpleRetry:
      pRetryObject.reset(new SimpleRetryObject(retry._retryIntervalMS));
      break;
    case LinearRetry:
      pRetryObject.reset(new LinearRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    case ExponentialRetry:
      pRetryObject.reset(new ExponentialRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    }
    bool hasUpdatedRetryCounterForCurrentTry;

    do {
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_COUNT");
      try {
	hasUpdatedRetryCounterForCurrentTry = false;
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
	dbcc = _conn.getInternals()->get()->query( 
						  _dbname+".$cmd", 
						  aQuery, 
						  1, 
						  0,
						  NULL,
						  0,
						  0
						   );
	//(aQuery) 
      }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
	pRetryObject->updateCounter();  /*update retry counter*/
	if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	  pRetryObject->waitUntilNextTry();
	  hasUpdatedRetryCounterForCurrentTry = true;
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
	}
	_conn.getInternals()->failover(); 
      }
      catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
	if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	  pRetryObject->updateCounter();  /*update retry counter*/
	  if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else {
	    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
	  }
	  _conn.getInternals()->failover(); 
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
	}
      }
       catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
      //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
	(iOperationType, QueryException, QueryError) 
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
      if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
	if (!hasUpdatedRetryCounterForCurrentTry)  { 
	  pRetryObject->updateCounter(); 
	  if (pRetryObject->shouldRetry()) { 
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else { 
	    DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
	  }
	}
	_conn.getInternals()->failover(); 
      }
      (iOperationType)
      else if (!dbcc->more()) {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
	if (!hasUpdatedRetryCounterForCurrentTry)  {
	  pRetryObject->updateCounter(); 
	  if (pRetryObject->shouldRetry()) { 
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else {
	    DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
	  }
	}
	_conn.getInternals()->failover(); 
      }
      //(iOperationType)
      else {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_COUNT_ERROR_OR_RETURN");
	mongo::BSONObj resultObj = dbcc->nextSafe().copy();
	if (!resultObj.hasElement("n")) {
	  DSCLIENT_THROW(QueryException, QueryError, "No nested 'n' element: " + resultObj.toString());
	}else {
	  return static_cast<uint64_t>(resultObj["n"].Number());;
	}
      }
      //(iOperationType) 
    }while(pRetryObject->shouldRetry());
    //(aQuery, iOperationType )
  }
 }while(0);

#define DSCLIENT_RETRY_OPERATION_DISTINCT(aQuery, iOperationType) 
do { 
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_OPERATION_DISTINCT");
  std::auto_ptr<mongo::DBClientCursor> dbcc;
  if (!option.retry()) { 
    try { 
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
      dbcc = _conn.getInternals()->get()->query( 
						_dbname+".$cmd", 
						aQuery, 
						1, 
						0,
						NULL,
						0,
						0
						 );
      //(aQuery) 
    }DSCLIENT_TRACE_DEBUG("DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW");
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(ConnectionException, mongo::SocketException, ConnectionFails, "Probably a connection Error during reconnection")
       catch(mongo::UserException& iEx) {  
	 if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
	 }
	 else {
	   __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
	 }
       }
    //(iOperationType, iExceptionType, iErrorCode)
     catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
    //(iExceptionType, mongo::DBException, iErrorCode, "Error occurred while executing " #iOperationType ".")
      //(iOperationType, QueryException, QueryError)
      if (!dbcc.get()) {
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }else if (!dbcc->more()) {
	DSCLIENT_THROW(ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
      }
      else {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DISTINCT_ERROR_OR_RETURN");
	mongo::BSONObj resultObj = dbcc->nextSafe().copy();
	if (!resultObj.hasElement("values")) {
	  DSCLIENT_THROW(QueryException, QueryError, "No nested 'values' element: " + resultObj.toString());
	}else {
	  return resultObj["values"].Obj().copy();
	}
      }
    //(iOperationType) 
  }
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP");
    Retry retry = option.retry().get();
    boost::shared_ptr<RetryObject> pRetryObject;
    Retry_pattern retry_pattern = retry._retry_pattern;
    switch(retry_pattern) {
    case SimpleRetry:
      pRetryObject.reset(new SimpleRetryObject(retry._retryIntervalMS));
      break;
    case LinearRetry:
      pRetryObject.reset(new LinearRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    case ExponentialRetry:
      pRetryObject.reset(new ExponentialRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
      break;
    }
    bool hasUpdatedRetryCounterForCurrentTry;

    do {
      DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_DISTINCT");
      try {
	hasUpdatedRetryCounterForCurrentTry = false;
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
	dbcc = _conn.getInternals()->get()->query( 
						  _dbname+".$cmd", 
						  aQuery, 
						  1, 
						  0,
						  NULL,
						  0,
						  0
						   );
	//(aQuery) 
      }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
	pRetryObject->updateCounter();  /*update retry counter*/
	if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	  pRetryObject->waitUntilNextTry();
	  hasUpdatedRetryCounterForCurrentTry = true;
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
	}
	_conn.getInternals()->failover(); 
      }
      catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
	if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
	  pRetryObject->updateCounter();  /*update retry counter*/
	  if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else {
	    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
	  }
	  _conn.getInternals()->failover(); 
	}else {
	  __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
	}
      }
       catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
      //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
	//(iOperationType, QueryException, QueryError) 
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
      if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
	if (!hasUpdatedRetryCounterForCurrentTry)  { 
	  pRetryObject->updateCounter(); 
	  if (pRetryObject->shouldRetry()) { 
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else { 
	    DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
	  }
	}
	_conn.getInternals()->failover(); 
      }
      //(iOperationType)
      else if (!dbcc->more()) {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
	if (!hasUpdatedRetryCounterForCurrentTry)  {
	  pRetryObject->updateCounter(); 
	  if (pRetryObject->shouldRetry()) { 
	    pRetryObject->waitUntilNextTry();
	    hasUpdatedRetryCounterForCurrentTry = true;
	  }else {
	    DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
	  }
	}
	_conn.getInternals()->failover(); 
      }
      //(iOperationType)
      else {
	DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DISTINCT_ERROR_OR_RETURN");
	mongo::BSONObj resultObj = dbcc->nextSafe().copy();
	if (!resultObj.hasElement("values")) {
	  DSCLIENT_THROW(QueryException, QueryError, "No nested 'values' element: " + resultObj.toString());
	}else {
	  return resultObj["values"].Obj().copy();
	}
      }
      //(iOperationType) 
    }while(pRetryObject->shouldRetry());
    //(aQuery, iOperationType )
  }
 }while(0);

################################################################################
################################################################################
################################################################################

#define DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP 
DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_SETUP");
Retry retry = option.retry().get();
boost::shared_ptr<RetryObject> pRetryObject;
Retry_pattern retry_pattern = retry._retry_pattern;
switch(retry_pattern) {
 case SimpleRetry:
   pRetryObject.reset(new SimpleRetryObject(retry._retryIntervalMS));
   break;
 case LinearRetry:
   pRetryObject.reset(new LinearRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
   break;
 case ExponentialRetry:
   pRetryObject.reset(new ExponentialRetryObject(retry._numberOfRetries, retry._retryIntervalMS));
   break;
 }
bool hasUpdatedRetryCounterForCurrentTry;

#define DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_WRITE(aQuery, iOperationType )
do {
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_WRITE");
  try {
    hasUpdatedRetryCounterForCurrentTry = false;
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
    dbcc = _conn.getInternals()->get()->query( 
					      _dbname+".$cmd", 
					      aQuery, 
					      1, 
					      0,
					      NULL,
					      0,
					      0
					       );
    //(aQuery) 
  }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
    pRetryObject->updateCounter();  /*update retry counter*/
    if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
      pRetryObject->waitUntilNextTry();
      hasUpdatedRetryCounterForCurrentTry = true;
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
    }
    _conn.getInternals()->failover(); 
  }
  catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
      pRetryObject->updateCounter();  /*update retry counter*/
      if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	__DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
      }
      _conn.getInternals()->failover(); 
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
    }
  }
   catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
  //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
    //(iOperationType, WriteException, WriteError) 
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
  if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
    if (!hasUpdatedRetryCounterForCurrentTry)  { 
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else { 
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }
    }
    _conn.getInternals()->failover(); 
  }
  (iOperationType)
  else if (!dbcc->more()) {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
    if (!hasUpdatedRetryCounterForCurrentTry)  {
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
      }
    }
    _conn.getInternals()->failover(); 
  }
  (iOperationType)
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_WRITE_ERROR_OR_RETURN");
    mongo::BSONObj writeResult = dbcc->nextSafe().copy();
    if (writeResult.hasElement("writeErrors") || writeResult.hasElement("writeConcernError") || writeResult.hasElement("errmsg")) {
      DSCLIENT_THROW(WriteException, WriteError, "Error while performing " #iOperationType ": " + writeResult.toString());
    }else {
      return writeResult;
    }
  }
  //(iOperationType) 
 }while(pRetryObject->shouldRetry());

#define DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_FIND(aQuery, iOperationType )
do {
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_FIND");
  try {
    hasUpdatedRetryCounterForCurrentTry = false;
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_FIND_QUERY");
    dbcc = _conn.getInternals()->get()->query(
					      _dbname+"."+_collname,
					      aQuery,
					      option.limit().get_value_or(0),
					      option.skip().get_value_or(0),
					      option.projection().get_ptr(),
					      (mongo::QueryOptions) queryOptions,
					      option.batchSize().get_value_or(0)
					      );
    //(aQuery) 
  }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
    pRetryObject->updateCounter();  /*update retry counter*/
    if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
      pRetryObject->waitUntilNextTry();
      hasUpdatedRetryCounterForCurrentTry = true;
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
    }
    _conn.getInternals()->failover(); 
  }
  catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
      pRetryObject->updateCounter();  /*update retry counter*/
      if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	__DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
      }
      _conn.getInternals()->failover(); 
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
    }
  }
   catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
  //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
    //(iOperationType, QueryException, QueryError) 
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
  if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
    if (!hasUpdatedRetryCounterForCurrentTry)  { 
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else { 
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }
    }
    _conn.getInternals()->failover(); 
  }
  //(iOperationType)
  else {
    DSCLIENT_TRACE_INFO("xxx");
    std::auto_ptr<Cursor> pcurs(new Cursor(dbcc));
    return pcurs;
  }
 }while(pRetryObject->shouldRetry());

#define DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_AGGREGATE(aQuery, iOperationType ) 
do {
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_AGGREGATE");
  try {
    hasUpdatedRetryCounterForCurrentTry = false;
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_AGGREGATE_QUERY");
    dbcc =_conn.getInternals()->get()->query(
					     _dbname+".$cmd",
					     aQuery,
					     1,
					     0,
					     NULL,
					     (mongo::QueryOptions) queryOptions,
					     0
					     );
    //(aQuery) 
  }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
    pRetryObject->updateCounter();  /*update retry counter*/
    if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
      pRetryObject->waitUntilNextTry();
      hasUpdatedRetryCounterForCurrentTry = true;
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
    }
    _conn.getInternals()->failover(); 
  }
  catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
      pRetryObject->updateCounter();  /*update retry counter*/
      if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	__DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
      }
      _conn.getInternals()->failover(); 
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
    }
  }
   catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
  //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
    //(iOperationType, QueryException, QueryError) 
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
  if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
    if (!hasUpdatedRetryCounterForCurrentTry)  { 
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else { 
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }
    }
    _conn.getInternals()->failover(); 
  }
  //(iOperationType)
  else {
    std::auto_ptr<Cursor> pcurs(new Cursor(dbcc));
    return pcurs;
  }
 }while(pRetryObject->shouldRetry());

#define DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_COUNT(aQuery, iOperationType )
do {
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_COUNT");
  try {
    hasUpdatedRetryCounterForCurrentTry = false;
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
    dbcc = _conn.getInternals()->get()->query( 
					      _dbname+".$cmd", 
					      aQuery, 
					      1, 
					      0,
					      NULL,
					      0,
					      0
					       );
    //(aQuery) 
  }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
    pRetryObject->updateCounter();  /*update retry counter*/
    if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
      pRetryObject->waitUntilNextTry();
      hasUpdatedRetryCounterForCurrentTry = true;
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
    }
    _conn.getInternals()->failover(); 
  }
  catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
      pRetryObject->updateCounter();  /*update retry counter*/
      if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	__DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
      }
      _conn.getInternals()->failover(); 
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
    }
  }
   catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
  //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
    //(iOperationType, QueryException, QueryError) 
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
  if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
    if (!hasUpdatedRetryCounterForCurrentTry)  { 
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else { 
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }
    }
    _conn.getInternals()->failover(); 
  }
  //(iOperationType)
  else if (!dbcc->more()) {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
    if (!hasUpdatedRetryCounterForCurrentTry)  {
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
      }
    }
    _conn.getInternals()->failover(); 
  }
  //(iOperationType)
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_COUNT_ERROR_OR_RETURN");
    mongo::BSONObj resultObj = dbcc->nextSafe().copy();
    if (!resultObj.hasElement("n")) {
      DSCLIENT_THROW(QueryException, QueryError, "No nested 'n' element: " + resultObj.toString());
    }else {
      return static_cast<uint64_t>(resultObj["n"].Number());;
    }
  }
  //(iOperationType) 
 }while(pRetryObject->shouldRetry());

#define DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_DISTINCT(aQuery, iOperationType )
do {
  DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_OPERATION_DISTINCT");
  try {
    hasUpdatedRetryCounterForCurrentTry = false;
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
    dbcc = _conn.getInternals()->get()->query( 
					      _dbname+".$cmd", 
					      aQuery, 
					      1, 
					      0,
					      NULL,
					      0,
					      0
					       );
    //(aQuery) 
  }catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
    pRetryObject->updateCounter();  /*update retry counter*/
    if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
      pRetryObject->waitUntilNextTry();
      hasUpdatedRetryCounterForCurrentTry = true;
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
    }
    _conn.getInternals()->failover(); 
  }
  catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
    if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
      pRetryObject->updateCounter();  /*update retry counter*/
      if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	__DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
      }
      _conn.getInternals()->failover(); 
    }else {
      __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
    }
  }
   catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
  //(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")
    //(iOperationType, QueryException, QueryError) 
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
  if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
    if (!hasUpdatedRetryCounterForCurrentTry)  { 
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else { 
	DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
      }
    }
    _conn.getInternals()->failover(); 
  }
  //(iOperationType)
  else if (!dbcc->more()) {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
    if (!hasUpdatedRetryCounterForCurrentTry)  {
      pRetryObject->updateCounter(); 
      if (pRetryObject->shouldRetry()) { 
	pRetryObject->waitUntilNextTry();
	hasUpdatedRetryCounterForCurrentTry = true;
      }else {
	DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
      }
    }
    _conn.getInternals()->failover(); 
  }
  //(iOperationType)
  else {
    DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DISTINCT_ERROR_OR_RETURN");
    mongo::BSONObj resultObj = dbcc->nextSafe().copy();
    if (!resultObj.hasElement("values")) {
      DSCLIENT_THROW(QueryException, QueryError, "No nested 'values' element: " + resultObj.toString());
    }else {
      return resultObj["values"].Obj().copy();
    }
  }
  //(iOperationType) 
 }while(pRetryObject->shouldRetry());

#define DSCLIENT_RETRY_INTERNAL_BLOCK_CATCH_MONGO_DBEXCEPTION(iOperationType, iExceptionType, iErrorCode) 
catch(mongo::SocketException& iEx) { /*Exception caught, entering retry process */ 
  pRetryObject->updateCounter();  /*update retry counter*/
  if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
    pRetryObject->waitUntilNextTry();
    hasUpdatedRetryCounterForCurrentTry = true;
  }else {
    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::SocketException, ConnectionFails, "Maximum number of retries is reached");
  }
  _conn.getInternals()->failover(); 
 }
 catch(mongo::UserException& iEx) { /*Exception caught, entering retry process */ 
   if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
     pRetryObject->updateCounter();  /*update retry counter*/
     if (pRetryObject->shouldRetry()) { /*check if throw final exception*/
       pRetryObject->waitUntilNextTry();
       hasUpdatedRetryCounterForCurrentTry = true;
     }else {
       __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Maximum number of retries is reached");
     }
     _conn.getInternals()->failover(); 
   }else {
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode,  "Error occurred while executing " #iOperationType "."); 
   }
 }
 catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
//(iExceptionType, mongo::DBException, iErrorCode,  "Error occurred while executing " #iOperationType ".")


#define DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER(iOperationType) 
DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NULL_POINTER");
if (!dbcc.get()) { /*null pointer, probably connection error, entering retry process*/ 
  if (!hasUpdatedRetryCounterForCurrentTry)  { 
    pRetryObject->updateCounter(); 
    if (pRetryObject->shouldRetry()) { 
      pRetryObject->waitUntilNextTry();
      hasUpdatedRetryCounterForCurrentTry = true;
    }else { 
      DSCLIENT_THROW(ConnectionException, ConnectionFails, "NULL cursor returned while performing " #iOperationType ". Most certainly a connection failure.");
    }
  }
  _conn.getInternals()->failover(); 
 }

#define DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR(iOperationType) 
 else if (!dbcc->more()) {
   DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_NO_RESULT_CURSOR");
   if (!hasUpdatedRetryCounterForCurrentTry)  {
     pRetryObject->updateCounter(); 
     if (pRetryObject->shouldRetry()) { 
       pRetryObject->waitUntilNextTry();
       hasUpdatedRetryCounterForCurrentTry = true;
     }else {
       DSCLIENT_THROW( ConnectionException, ConnectionFails , "No write result returned from server side for " #iOperationType ". Probably a connection failure");
     }
   }
   _conn.getInternals()->failover(); 
 }

#define DSCLIENT_RETRY_INTERNAL_BLOCK_WRITE_ERROR_OR_RETURN(iOperationType) 
 else {
   DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_WRITE_ERROR_OR_RETURN");
   mongo::BSONObj writeResult = dbcc->nextSafe().copy();
   if (writeResult.hasElement("writeErrors") || writeResult.hasElement("writeConcernError") || writeResult.hasElement("errmsg")) {
     DSCLIENT_THROW(WriteException, WriteError, "Error while performing " #iOperationType ": " + writeResult.toString());
   }else {
     return writeResult;
   }
 }

#define DSCLIENT_RETRY_INTERNAL_BLOCK_COUNT_ERROR_OR_RETURN(iOperationType) 
 else {
   DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_COUNT_ERROR_OR_RETURN");
   mongo::BSONObj resultObj = dbcc->nextSafe().copy();
   if (!resultObj.hasElement("n")) {
     DSCLIENT_THROW(QueryException, QueryError, "No nested 'n' element: " + resultObj.toString());
   }else {
     return static_cast<uint64_t>(resultObj["n"].Number());;
   }
 }

#define DSCLIENT_RETRY_INTERNAL_BLOCK_DISTINCT_ERROR_OR_RETURN(iOperationType) 
 else {
   DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DISTINCT_ERROR_OR_RETURN");
   mongo::BSONObj resultObj = dbcc->nextSafe().copy();
   if (!resultObj.hasElement("values")) {
     DSCLIENT_THROW(QueryException, QueryError, "No nested 'values' element: " + resultObj.toString());
   }else {
     return resultObj["values"].Obj().copy();
   }
 }

#define DSCLIENT_RETRY_INTERNAL_BLOCK_FIND_QUERY(aQuery) 
DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_FIND_QUERY");
dbcc = _conn.getInternals()->get()->query(
					  _dbname+"."+_collname,
					  aQuery,
					  option.limit().get_value_or(0),
					  option.skip().get_value_or(0),
					  option.projection().get_ptr(),
					  (mongo::QueryOptions) queryOptions,
					  option.batchSize().get_value_or(0)
					  );

#define DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY(aQuery) 
DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_DBCOMMAND_QUERY");
dbcc = _conn.getInternals()->get()->query( 
					  _dbname+".$cmd", 
					  aQuery, 
					  1, 
					  0,
					  NULL,
					  0,
					  0
					   );

#define DSCLIENT_RETRY_INTERNAL_BLOCK_AGGREGATE_QUERY(aQuery) 
DSCLIENT_TRACE_DEBUG("DSCLIENT_RETRY_INTERNAL_BLOCK_AGGREGATE_QUERY");
dbcc =_conn.getInternals()->get()->query(
					 _dbname+".$cmd",
					 aQuery,
					 1,
					 0,
					 NULL,
					 (mongo::QueryOptions) queryOptions,
					 0
					 );

#define DSCLIENT_CONNECTION_FAILOVER
DSCLIENT_TRACE_DEBUG("DSCLIENT_CONNECTION_FAILOVER");
if (!_conn.isStillConnected()) {
  _conn.getInternals()->failover();
 }

#define DSCLIENT_BULKINSERT_RESULT_BUILD 
DSCLIENT_TRACE_DEBUG("DSCLIENT_BULKINSERT_RESULT_BUILD");
if (!wr.hasErrors()) { 
  DSCLIENT_TRACE_DEBUG("No write error for bulk insert"); 
  return BSON("nInserted"<<wr.nInserted());
 }else {
  mongo::BSONObjBuilder resultBuilder;
  resultBuilder<<"nInserted"<<wr.nInserted();
  if (wr.hasWriteErrors()) {
    std::vector<mongo::BSONObj> writeErrors = wr.writeErrors();
    mongo::BSONArrayBuilder writeErrorsArrayBuilder;
    for(std::vector<mongo::BSONObj>::const_iterator it = writeErrors.begin() ; it != writeErrors.end(); ++it) {
      writeErrorsArrayBuilder<<(*it);
    }
    resultBuilder<<"WriteErrors"<<writeErrorsArrayBuilder.arr();
  }
  if (wr.hasWriteConcernErrors()) {
    std::vector<mongo::BSONObj> writeConcernErrors = wr.writeConcernErrors();
    mongo::BSONArrayBuilder writeConcernErrorsArrayBuilder;
    for(std::vector<mongo::BSONObj>::const_iterator it = writeConcernErrors.begin() ; it != writeConcernErrors.end(); ++it) {
      writeConcernErrorsArrayBuilder<<(*it);
    }
    resultBuilder<<"WriteConcernErrors"<<writeConcernErrorsArrayBuilder.arr();
  }
  return resultBuilder.obj();
 }


#define DSCLIENT_CATCH_MONGO_USER_EXCEPTION_10276_RETHROW(iOperationType, iExceptionType, iErrorCode) 
catch(mongo::UserException& iEx) {  
  if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
  }
  else {
    __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
  }
 }


#define DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW(iOperationType, iExceptionType, iErrorCode)
DSCLIENT_TRACE_DEBUG("DSCLIENT_CATCH_ALL_CONNECTION_EXCEPTION_RETHROW");
 catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
//(ConnectionException, mongo::SocketException, ConnectionFails, "Probably a connection Error during reconnection")
catch(mongo::UserException& iEx) {  
  if (iEx.getCode()==10276) { /* 10276 code DBClientBase::findN: transport error: ns: query: */
    __DSCLIENT_RETHROW_INTERNAL_BLOCK( ConnectionException, mongo::UserException, ConnectionFails, "Probably a connection failure");
  }
  else {
    __DSCLIENT_RETHROW_INTERNAL_BLOCK( iExceptionType, mongo::UserException, iErrorCode, "Error occurred while executing " #iOperationType "."); 
  }
 }
//(iOperationType, iExceptionType, iErrorCode)
 catch( iExcType& iEx ) 
   { 
     __DSCLIENT_RETHROW_INTERNAL_BLOCK( iDSClientExceptionType, iExcType, iErrorCode, iMessage ); 
   }
//(iExceptionType, mongo::DBException, iErrorCode, "Error occurred while executing " #iOperationType ".")


#endif /* COLLECTIONIMPLMACROS_H_ */
