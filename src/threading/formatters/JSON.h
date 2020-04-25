// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#define RAPIDJSON_HAS_STDSTRING 1
#include "3rdparty/rapidjson/include/rapidjson/document.h"
#include "3rdparty/rapidjson/include/rapidjson/writer.h"

#include "../Formatter.h"

namespace threading { namespace formatter {

/**
  * A thread-safe class for converting values into a JSON representation
  * and vice versa.
  */
class JSON : public Formatter {
public:
	enum TimeFormat {
		TS_EPOCH,	// Doubles that represents seconds from the UNIX epoch.
		TS_ISO8601,	// ISO 8601 defined human readable timestamp format.
		TS_MILLIS	// Milliseconds from the UNIX epoch.  Some consumers need this (e.g., elasticsearch).
		};

	/**
	 * Constructor.
	 *
	 * @param t The thread that uses this class instance. The class uses
	 * some of the thread's methods, e.g., for error reporting and
	 * internal formatting.
	 *
	 * @param tf TimeFormat The format to use for time fields.
	 *
	 * @param size_limit_hint Specifies a maximum size that shouldn't be
	 * significantly exceeded for the final JSON representation of a log
	 * entry. If necessary the formatter will truncate the data. It's not
	 * a hard limit though, the result might still be slightly larger
	 * than the limit. It will remain a syntactically valid log entry,
	 * even if truncated. Set to zero to disable any truncation.
	 */
	JSON(threading::MsgThread* t, TimeFormat tf, unsigned int size_limit_hint = 0);
	~JSON() override;

	bool Describe(ODesc* desc, threading::Value* val, const std::string& name = "") const override;
	bool Describe(ODesc* desc, int num_fields, const threading::Field* const * fields,
	                      threading::Value** vals) const override;
	threading::Value* ParseValue(const std::string& s, const std::string& name, zeek::TypeTag type, zeek::TypeTag subtype = zeek::TYPE_ERROR) const override;

	class ZeekWriter : public rapidjson::Writer<rapidjson::StringBuffer> {
	public:
		ZeekWriter(rapidjson::StringBuffer& stream) : rapidjson::Writer<rapidjson::StringBuffer>(stream) {}
		bool Double(double d);

                /**
                 * Helper to make the built-up size of the underlying
                 * string buffer available directly from the writer.
                 * This is a simple offset calculation that runs in
                 * constant time.
                 */
                size_t GetBufferSize() const { return os_->GetSize(); }

                /**
                 * Records the fact that a member of the given name
                 * got truncated during writing.
                 */
                void AddTruncation(const string& name) { truncated.push_back(name); }

                /**
                 * Writes out any recorded truncations. The return
                 * value is true if the entirety of the writes
                 * succeeded, and false when any of those writes
                 * failed. In such a case the call returns
                 * immediately.
                 */
                bool Truncations();

        protected:
                std::vector<string> truncated;
	};

private:
	void BuildJSON(ZeekWriter& writer, Value* val, const string& name = "",
		       const string& last_name = "") const;

	TimeFormat timestamps;
	bool surrounding_braces;
	unsigned int size_limit_hint;
};

}}
