/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef FILE_IO_VISITOR_H
#define FILE_IO_VISITOR_H

#include <file_reader.h>
#include <file_writer.h>

class FileReaderVisitor
{
protected:
   FileReader reader;

public:
   FileReaderVisitor(File* file)
      : reader()
   {
      reader.init(file);
   }

   bool good() const { return reader.good(); }

   void with_record_size(uint16_t expected_record_size)
   {
      reader.check_record_size(expected_record_size);
   }

   bool skip(size_t len)
   {
      return reader.skip(len);
   }

   template <typename FileT, typename MemT>
   bool visit(MemT *v)
   {
      return reader.read<FileT, MemT>(v);
   }

   template <typename T>
   bool visit(T **v)
   {
      uint32_t p;
      if (!reader.read<uint32_t>(&p))
         return false;

      if (p != 0)
         *v = reinterpret_cast<T *>(uintptr_t(0xdeadbeefUL));
      else
         *v = NULL;

      return true;
   }
};


class FileWriterVisitor
{
protected:
   FileWriter writer;

public:
   FileWriterVisitor(File* file)
      : writer()
   {
      writer.init(file);
   }

   bool good() const { return writer.good(); }

   void with_record_size(uint16_t record_size)
   {
      writer.write_record_size(record_size);
   }

   bool skip(size_t len)
   {
      return writer.skip(len);
   }

   template <typename FileT, typename MemT>
   bool visit(MemT *v)
   {
      return writer.write<FileT, MemT>(*v);
   }

   template <typename T>
   bool visit(T **v)
   {
      uint32_t p (*v ? 0xdeadbeefUL : 0);
      return writer.write<uint32_t>(p);
   }
};

namespace FileIOVisitor
{
   template <typename FileT, typename MemT, typename Visitor>
   bool visit(Visitor *vis, MemT *val)
   {
      return vis->template visit<FileT, MemT>(val);
   }

   template <typename FileT, typename MemT, typename Visitor, int Size>
   bool visit_array(Visitor *vis, MemT (&array)[Size])
   {
      for (int i = 0; i < Size; ++i)
      {
         if (!vis->template visit<FileT, MemT>(&array[i]))
            return false;
      }
      return true;
   }

   template <typename T, typename Visitor>
   bool visit_pointer(Visitor *vis, T **ptr)
   {
      return vis->visit(ptr);
   }

   template <typename Visitor, typename T>
   bool visit_with_record_size(File* file, T* obj, void (*visit_obj)(Visitor* v, T* obj), uint16_t rec_size)
   {
      Visitor v(file);
      v.with_record_size(rec_size);
      visit_obj(&v, obj);

      return v.good();
   }

} /* namespace FileIOVisitor */

/* vim: set ts=8 sw=3: */
#endif
