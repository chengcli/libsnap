//! \file io_wrapper.cpp
//! \brief functions that provide wrapper for MPI-IO versus serial input/output

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

// base
#include <configure.h>

#include "io_wrapper.hpp"

namespace snap {
//! \brief wrapper for {MPI_File_open} versus {std::fopen} including error check
int IOWrapper::Open(const char *fname, FileMode rw) {
  std::stringstream msg;

  if (rw == FileMode::read) {
#ifdef MPI_PARALLEL
    // NOLINTNEXTLINE
    if (MPI_File_open(comm_, const_cast<char *>(fname), MPI_MODE_RDONLY,
                      MPI_INFO_NULL, &fh_) !=
        MPI_SUCCESS)  // use const_cast to convince the compiler.
#else
    if ((fh_ = std::fopen(fname, "rb")) == nullptr)  // NOLINT
#endif
    {
      msg << "### FATAL ERROR in function [IOWrapper:Open]" << std::endl
          << "Input file '" << fname << "' could not be opened" << std::endl;
      throw std::runtime_error(msg.str().c_str());
      return false;
    }

  } else if (rw == FileMode::write) {
#ifdef MPI_PARALLEL
    MPI_File_delete(const_cast<char *>(fname), MPI_INFO_NULL);  // truncation
    // NOLINTNEXTLINE
    if (MPI_File_open(comm_, const_cast<char *>(fname),
                      MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL,
                      &fh_) != MPI_SUCCESS)
#else
    if ((fh_ = std::fopen(fname, "wb")) == nullptr)  // NOLINT
#endif
    {
      msg << "### FATAL ERROR in function [IOWrapper:Open]" << std::endl
          << "Output file '" << fname << "' could not be opened" << std::endl;
      throw std::runtime_error(msg.str().c_str());
      return false;
    }
  } else {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Read(void *buf, IOWrapperSizeT size, IOWrapperSizeT
//! count) \brief wrapper for {MPI_File_read} versus {std::fread}

std::size_t IOWrapper::Read(void *buf, IOWrapperSizeT size,
                            IOWrapperSizeT count) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nread;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Read]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_read(fh_, buf, count * size, MPI_BYTE, &status) != MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nread) == MPI_UNDEFINED) return -1;
  return nread / size;
#else
  return std::fread(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Read_all(void *buf, IOWrapperSizeT size, IOWrapperSizeT
//! count) \brief wrapper for {MPI_File_read_all} versus {std::fread}

std::size_t IOWrapper::Read_all(void *buf, IOWrapperSizeT size,
                                IOWrapperSizeT count) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nread;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Read_all]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_read_all(fh_, buf, count * size, MPI_BYTE, &status) !=
      MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nread) == MPI_UNDEFINED) return -1;
  return nread / size;
#else
  return std::fread(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Read_at(void *buf, IOWrapperSizeT size,
//!                            IOWrapperSizeT count, IOWrapperSizeT offset)
//! \brief wrapper for {MPI_File_read_at} versus {std::fseek+std::fread}

std::size_t IOWrapper::Read_at(void *buf, IOWrapperSizeT size,
                               IOWrapperSizeT count, IOWrapperSizeT offset) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nread;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Read_at]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_read_at(fh_, offset, buf, count * size, MPI_BYTE, &status) !=
      MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nread) == MPI_UNDEFINED) return -1;
  return nread / size;
#else
  std::fseek(fh_, offset, SEEK_SET);
  return std::fread(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Read_at_all(void *buf, IOWrapperSizeT size,
//!                                IOWrapperSizeT count, IOWrapperSizeT offset)
//! \brief wrapper for {MPI_File_read_at_all} versus {std::fseek+std::fread}

std::size_t IOWrapper::Read_at_all(void *buf, IOWrapperSizeT size,
                                   IOWrapperSizeT count,
                                   IOWrapperSizeT offset) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nread;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Read_at_all]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_read_at_all(fh_, offset, buf, count * size, MPI_BYTE, &status) !=
      MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nread) == MPI_UNDEFINED) return -1;
  return nread / size;
#else
  std::fseek(fh_, offset, SEEK_SET);
  return std::fread(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Write(const void *buf, IOWrapperSizeT size,
//! IOWrapperSizeT count) \brief wrapper for {MPI_File_write} versus
//! {std::fwrite}

std::size_t IOWrapper::Write(const void *buf, IOWrapperSizeT size,
                             IOWrapperSizeT count) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nwrite;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Write]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_write(fh_, const_cast<void *>(buf), count * size, MPI_BYTE,
                     &status) != MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nwrite) == MPI_UNDEFINED) return -1;
  return nwrite / size;
#else
  return std::fwrite(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Write_at(const void *buf, IOWrapperSizeT size,
//!                                 IOWrapperSizeT count, IOWrapperSizeT offset)
//! \brief wrapper for {MPI_File_write_at} versus {std::fseek+std::fwrite}.

std::size_t IOWrapper::Write_at(const void *buf, IOWrapperSizeT size,
                                IOWrapperSizeT count, IOWrapperSizeT offset) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nwrite;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Write_at]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_write_at(fh_, offset, const_cast<void *>(buf), count * size,
                        MPI_BYTE, &status) != MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nwrite) == MPI_UNDEFINED) return -1;
  return nwrite / size;
#else
  std::fseek(fh_, offset, SEEK_SET);
  return std::fwrite(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Write_at_all(const void *buf, IOWrapperSizeT size,
//!                                 IOWrapperSizeT count, IOWrapperSizeT offset)
//! \brief wrapper for {MPI_File_write_at_all} versus {std::fseek+std::fwrite}.

std::size_t IOWrapper::Write_at_all(const void *buf, IOWrapperSizeT size,
                                    IOWrapperSizeT count,
                                    IOWrapperSizeT offset) {
#ifdef MPI_PARALLEL
  MPI_Status status;
  int nwrite;
  if (count * size > std::numeric_limits<int>::max()) {
    std::stringstream msg;
    msg << "### FATAL ERROR in function [IOWrapper:Write_at_all]"
        << "The data size is too large. size * count = " << count * size
        << " > " << std::numeric_limits<int>::max() << std::endl;
    ;
    throw std::runtime_error(msg.str().c_str());
    return false;
  }
  if (MPI_File_write_at_all(fh_, offset, const_cast<void *>(buf), count * size,
                            MPI_BYTE, &status) != MPI_SUCCESS)
    return -1;
  if (MPI_Get_count(&status, MPI_BYTE, &nwrite) == MPI_UNDEFINED) return -1;
  return nwrite / size;
#else
  std::fseek(fh_, offset, SEEK_SET);
  return std::fwrite(buf, size, count, fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn void IOWrapper::Close()
//! \brief wrapper for {MPI_File_close} versus {std::fclose}

int IOWrapper::Close() {
#ifdef MPI_PARALLEL
  return MPI_File_close(&fh_);
#else
  return std::fclose(fh_);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn int IOWrapper::Seek(IOWrapperSizeT offset)
//! \brief wrapper for {MPI_File_seek} versus {std::fseek}

int IOWrapper::Seek(IOWrapperSizeT offset) {
#ifdef MPI_PARALLEL
  return MPI_File_seek(fh_, offset, MPI_SEEK_SET);
#else
  return std::fseek(fh_, offset, SEEK_SET);
#endif
}

//----------------------------------------------------------------------------------------
//! \fn IOWrapperSizeT IOWrapper::GetPosition()
//! \brief wrapper for {MPI_File_get_position} versus {ftell}

IOWrapperSizeT IOWrapper::GetPosition() {
#ifdef MPI_PARALLEL
  MPI_Offset position;
  MPI_File_get_position(fh_, &position);
  return position;
#else
  return ftell(fh_);
#endif
}
}  // namespace snap
