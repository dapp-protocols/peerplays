#pragma once
#include <fc/config.hpp>
#include <fc/time.hpp>
#include <fc/log/appender.hpp>
#include <fc/log/log_message.hpp>
#include <cstddef>
#include <memory>

namespace fc
{
   /**
    *
    *
    @code
      void my_class::func()
      {
         fc_dlog( my_class_logger, "Format four: ${arg}  five: ${five}", ("arg",4)("five",5) );
      }
    @endcode
    */
   class logger
   {
      public:
         static logger get( const std::string& name = "default");

         logger();
         logger( const string& name, const logger& parent = nullptr );
         logger( std::nullptr_t );
         logger( const logger& c );
         logger( logger&& c );
         ~logger();
         logger& operator=(const logger&);
         logger& operator=(logger&&);
         friend bool operator==( const logger&, std::nullptr_t );
         friend bool operator!=( const logger&, std::nullptr_t );

         logger&    set_log_level( log_level e );
         log_level  get_log_level()const;
         logger&    set_parent( const logger& l );
         logger     get_parent()const;

         void  set_name( const std::string& n );
         const std::string& name()const;

         void add_appender( const appender::ptr& a );
         std::vector<appender::ptr> get_appenders()const;
         void remove_appender( const appender::ptr& a );

         bool is_enabled( log_level e )const;
         void log( log_message m );

      private:
         class impl;
         std::shared_ptr<impl> my;
   };

} // namespace fc

#ifndef DEFAULT_LOGGER
#define DEFAULT_LOGGER
#endif

// suppress warning "conditional expression is constant" in the while(0) for visual c++
// http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/
#define FC_MULTILINE_MACRO_BEGIN do {
#ifdef _MSC_VER
# define FC_MULTILINE_MACRO_END \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127)) \
    } while (0) \
    __pragma(warning(pop))
#else
# define FC_MULTILINE_MACRO_END  } while (0)
#endif

#define fc_dlog( LOGGER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fc::log_level::debug ) ) \
      (LOGGER).log( FC_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#define fc_ilog( LOGGER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fc::log_level::info ) ) \
      (LOGGER).log( FC_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#define fc_wlog( LOGGER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fc::log_level::warn ) ) \
      (LOGGER).log( FC_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#define fc_elog( LOGGER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (LOGGER).is_enabled( fc::log_level::error ) ) \
      (LOGGER).log( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#define dlog( FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (fc::logger::get(DEFAULT_LOGGER)).is_enabled( fc::log_level::debug ) ) \
      (fc::logger::get(DEFAULT_LOGGER)).log( FC_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

/**
 * Sends the log message to a special 'user' log stream designed for messages that
 * the end user may like to see.
 */
#define ulog( FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (fc::logger::get("user")).is_enabled( fc::log_level::debug ) ) \
      (fc::logger::get("user")).log( FC_LOG_MESSAGE( debug, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END


#define ilog( FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (fc::logger::get(DEFAULT_LOGGER)).is_enabled( fc::log_level::info ) ) \
      (fc::logger::get(DEFAULT_LOGGER)).log( FC_LOG_MESSAGE( info, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#define wlog( FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (fc::logger::get(DEFAULT_LOGGER)).is_enabled( fc::log_level::warn ) ) \
      (fc::logger::get(DEFAULT_LOGGER)).log( FC_LOG_MESSAGE( warn, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#define elog( FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( (fc::logger::get(DEFAULT_LOGGER)).is_enabled( fc::log_level::error ) ) \
      (fc::logger::get(DEFAULT_LOGGER)).log( FC_LOG_MESSAGE( error, FORMAT, __VA_ARGS__ ) ); \
  FC_MULTILINE_MACRO_END

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>


#define FC_FORMAT_ARG(r, unused, base) \
  BOOST_PP_STRINGIZE(base) ": ${" BOOST_PP_STRINGIZE( base ) "} "

#define FC_FORMAT_ARGS(r, unused, base) \
  BOOST_PP_LPAREN() BOOST_PP_STRINGIZE(base),fc::variant(base,FC_MAX_LOG_OBJECT_DEPTH) BOOST_PP_RPAREN()

#define FC_FORMAT( SEQ )\
    BOOST_PP_SEQ_FOR_EACH( FC_FORMAT_ARG, v, SEQ )

// takes a ... instead of a SEQ arg because it can be called with an empty SEQ
// from FC_CAPTURE_AND_THROW()
#define FC_FORMAT_ARG_PARAMS( ... )\
    BOOST_PP_SEQ_FOR_EACH( FC_FORMAT_ARGS, v, __VA_ARGS__ )

#define FC_DUMP_FORMAT_ARG_NAME(r, unused, base) \
   "(" BOOST_PP_STRINGIZE(base) ")"

#define FC_DUMP_FORMAT_ARG_NAMES( SEQ )\
   BOOST_PP_SEQ_FOR_EACH( FC_DUMP_FORMAT_ARG_NAME, v, SEQ )

#define fc_ddump( LOGGER, SEQ ) \
    fc_dlog( LOGGER, FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  
#define fc_idump( LOGGER, SEQ ) \
    fc_ilog( LOGGER, FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  
#define fc_wdump( LOGGER, SEQ ) \
    fc_wlog( LOGGER, FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  
#define fc_edump( LOGGER, SEQ ) \
    fc_elog( LOGGER, FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  

#define ddump( SEQ ) \
    dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  

// TODO FC_FORMAT_ARG_PARAMS(...) may throw exceptions when calling fc::variant(...) inside,
//      as a quick-fix / workaround, we catch all exceptions here.
//      However, to log as much info as possible, it's better to catch exceptions when processing each argument
#define idump( SEQ ) \
{ \
   try { \
      ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) ); \
   } catch( ... ) { \
      ilog ( "[ERROR: Got exception while trying to dump ( ${args} )]",("args",FC_DUMP_FORMAT_ARG_NAMES(SEQ)) ); \
   } \
}
#define wdump( SEQ ) \
{ \
   try { \
      wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) ); \
   } catch( ... ) { \
      wlog ( "[ERROR: Got exception while trying to dump ( ${args} )]",("args",FC_DUMP_FORMAT_ARG_NAMES(SEQ)) ); \
   } \
}
#define edump( SEQ ) \
{ \
   try { \
      elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) ); \
   } catch( ... ) { \
      elog ( "[ERROR: Got exception while trying to dump ( ${args} )]",("args",FC_DUMP_FORMAT_ARG_NAMES(SEQ)) ); \
   } \
}

// this disables all normal logging statements -- not something you'd normally want to do,
// but it's useful if you're benchmarking something and suspect logging is causing
// a slowdown.
#ifdef FC_DISABLE_LOGGING
# undef ulog
# define ulog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef elog
# define elog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef wlog
# define wlog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef ilog
# define ilog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef dlog
# define dlog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
#endif
