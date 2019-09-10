
#include "FileReassembler.h"
#include "File.h"


namespace file_analysis {

class File;

FileReassembler::FileReassembler(File *f, uint64_t starting_offset)
	: Reassembler(starting_offset, REASSEM_FILE), the_file(f), flushing(false)
	{
	}

FileReassembler::FileReassembler()
	: Reassembler(), the_file(0), flushing(false)
	{
	}

FileReassembler::~FileReassembler()
	{
	}

uint64_t FileReassembler::Flush()
	{
	if ( flushing )
		return 0;

	auto last_block = block_list.Tail();

	if ( last_block )
		{
		// This is expected to call back into FileReassembler::Undelivered().
		flushing = true;
		uint64_t rval = TrimToSeq(last_block->db->upper);
		flushing = false;
		return rval;
		}

	return 0;
	}

uint64_t FileReassembler::FlushTo(uint64_t sequence)
	{
	if ( flushing )
		return 0;

	flushing = true;
	uint64_t rval = TrimToSeq(sequence);
	flushing = false;
	last_reassem_seq = sequence;
	return rval;
	}

void FileReassembler::BlockInserted(const DataBlockNode* start_block)
	{
	if ( start_block->db->seq > last_reassem_seq ||
	     start_block->db->upper <= last_reassem_seq )
		return;

	// TODO: better way to iterate ?
	for ( auto b = start_block;
	      b && b->db->seq <= last_reassem_seq; b = b->next )
		{
		if ( b->db->seq == last_reassem_seq )
			{ // New stuff.
			uint64_t len = b->Size();
			last_reassem_seq += len;
			the_file->DeliverStream(b->db->block, len);
			}
		}

	// Throw out forwarded data
	TrimToSeq(last_reassem_seq);
	}

void FileReassembler::Undelivered(uint64_t up_to_seq)
	{
	// If we have blocks that begin below up_to_seq, deliver them.
	const DataBlockNode* b = block_list.Head();

	// TODO: better way to iterate ?
	while ( b )
		{
		if ( b->db->seq < last_reassem_seq )
			{
			// Already delivered this block.
			b = b->next;
			continue;
			}

		if ( b->db->seq >= up_to_seq )
			// Block is beyond what we need to process at this point.
			break;

		uint64_t gap_at_seq = last_reassem_seq;
		uint64_t gap_len = b->db->seq - last_reassem_seq;
		the_file->Gap(gap_at_seq, gap_len);
		last_reassem_seq += gap_len;
		BlockInserted(b);
		// Inserting a block may cause trimming of what's buffered,
		// so have to assume 'b' is invalid, hence re-assign to start.
		b = block_list.Head();
		}

	if ( up_to_seq > last_reassem_seq )
		{
		the_file->Gap(last_reassem_seq, up_to_seq - last_reassem_seq);
		last_reassem_seq = up_to_seq;
		}
	}

void FileReassembler::Overlap(const u_char* b1, const u_char* b2, uint64_t n)
	{
	// Not doing anything here yet.
	}
} // end file_analysis
