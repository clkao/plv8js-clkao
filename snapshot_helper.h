#include <v8.h>
#include <v8-profiler.h>

class FileOutputStream : public v8::OutputStream {
   public:
      FileOutputStream(FILE* stream) : stream_(stream) {}

      virtual int GetChunkSize() {
	 return 65536; // big chunks == faster
      }

      virtual void EndOfStream() {}

      virtual WriteResult WriteAsciiChunk(char* data, int size) {
	 const size_t len = static_cast<size_t>(size);
	 size_t off = 0;
	 while (off < len && !feof(stream_) && !ferror(stream_))
	    off += fwrite(data + off, 1, len - off, stream_);
	 return off == len ? kContinue : kAbort;
      }

   private:
      FILE* stream_;
};

inline bool WriteSnapshotHelper(v8::Isolate* isolate, const char* filename) {
   FILE* fp = fopen(filename, "w");
   if (fp == NULL) return false;
   const v8::HeapSnapshot* const snap = plv8_isolate->GetHeapProfiler()->TakeHeapSnapshot(
      v8::String::NewFromUtf8(plv8_isolate, "plv8 Heap Snapshot"));
   FileOutputStream stream(fp);
   snap->Serialize(&stream, v8::HeapSnapshot::kJSON);
   fclose(fp);
   plv8_isolate->GetHeapProfiler()->DeleteAllHeapSnapshots();
   return true;
}
