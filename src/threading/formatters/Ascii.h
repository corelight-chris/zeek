// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include "../Formatter.h"

namespace threading { namespace formatter {

class Ascii final : public Formatter {
public:
	/**
	 * A struct to pass the necessary configuration values to the
	 * Ascii module on initialization.
	 */
	struct SeparatorInfo
		{
		std::string separator;		// Separator between columns
		std::string set_separator;	// Separator between set elements.
		std::string unset_field;	// String marking an unset field.
		std::string empty_field;	// String marking an empty (but set) field.

		/**
		 * Constructor that defines all the configuration options.
		 * Use if you need either ValToODesc or EntryToVal.
		 */
		SeparatorInfo(const std::string& separator, const std::string& set_separator,
		              const std::string& unset_field, const std::string& empty_field);

		/**
		 * Constructor that leaves separators etc unset to dummy
		 * values. Useful if you use only methods that don't need any
		 * of them, like StringToAddr, etc.
		 */
		SeparatorInfo();
		};

	/**
	 * Constructor.
	 *
	 * @param t The thread that uses this class instance. The class uses
	 * some of the thread's methods, e.g., for error reporting and
	 * internal formatting.
	 *
	 * @param info SeparatorInfo structure defining the necessary
	 * separators.
	 *
	 * #@param size_limit_hint Specifies a maximum size that shouldn't be
	 * significantly exceeded for the final ASCII representation of a log
	 * entry. If necessary the formatter will truncate the data. It's not
	 * a hard limit though, the result might still be slightly larger
	 * than the limit. It will remain a syntactically valid log entry,
	 * even if truncated. Set to zero to disable any truncation.
	 */
	Ascii(threading::MsgThread* t, const SeparatorInfo& info, unsigned int size_limit_hint = 0);
	virtual ~Ascii();

	virtual bool Describe(ODesc* desc, threading::Value* val, const std::string& name = "") const;
	virtual bool Describe(ODesc* desc, int num_fields, const threading::Field* const * fields,
	                      threading::Value** vals) const;
	virtual threading::Value* ParseValue(const std::string& s, const std::string& name,
		                                 zeek::TypeTag type, zeek::TypeTag subtype = zeek::TYPE_ERROR) const;

private:
	bool CheckNumberError(const char* start, const char* end) const;

	SeparatorInfo separators;
	unsigned int size_limit_hint;
};

}}
