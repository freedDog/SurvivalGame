// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <iostream>
#include <stack>
#include <unordered_map>

#include <google/protobuf/message.h>

#include <google/protobuf/stubs/casts.h>
#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/reflection_internal.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/map_field.h>
#include <google/protobuf/map_field_inl.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/stubs/strutil.h>

#include <google/protobuf/stubs/map_util.h>
#include <google/protobuf/stubs/stl_util.h>
#include <google/protobuf/stubs/hash.h>

#include <google/protobuf/port_def.inc>

namespace google {
namespace protobuf {

#if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
using internal::ParseClosure;
#endif
using internal::ReflectionOps;
using internal::WireFormat;
using internal::WireFormatLite;

void Message::MergeFrom(const Message& from) {
  const Descriptor* descriptor = GetDescriptor();
  GOOGLE_CHECK_EQ(from.GetDescriptor(), descriptor)
    << ": Tried to merge from a message with a different type.  "
       "to: " << descriptor->full_name() << ", "
       "from: " << from.GetDescriptor()->full_name();
  ReflectionOps::Merge(from, this);
}

void Message::CheckTypeAndMergeFrom(const MessageLite& other) {
  MergeFrom(*down_cast<const Message*>(&other));
}

void Message::CopyFrom(const Message& from) {
  const Descriptor* descriptor = GetDescriptor();
  GOOGLE_CHECK_EQ(from.GetDescriptor(), descriptor)
    << ": Tried to copy from a message with a different type. "
       "to: " << descriptor->full_name() << ", "
       "from: " << from.GetDescriptor()->full_name();
  ReflectionOps::Copy(from, this);
}

string Message::GetTypeName() const {
  return GetDescriptor()->full_name();
}

void Message::Clear() {
  ReflectionOps::Clear(this);
}

bool Message::IsInitialized() const {
  return ReflectionOps::IsInitialized(*this);
}

void Message::FindInitializationErrors(std::vector<string>* errors) const {
  return ReflectionOps::FindInitializationErrors(*this, "", errors);
}

string Message::InitializationErrorString() const {
  std::vector<string> errors;
  FindInitializationErrors(&errors);
  return Join(errors, ", ");
}

void Message::CheckInitialized() const {
  GOOGLE_CHECK(IsInitialized())
    << "Message of type \"" << GetDescriptor()->full_name()
    << "\" is missing required fields: " << InitializationErrorString();
}

void Message::DiscardUnknownFields() {
  return ReflectionOps::DiscardUnknownFields(this);
}

#if !GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
bool Message::MergePartialFromCodedStream(io::CodedInputStream* input) {
  return WireFormat::ParseAndMergePartial(input, this);
}
#endif

bool Message::ParseFromFileDescriptor(int file_descriptor) {
  io::FileInputStream input(file_descriptor);
  return ParseFromZeroCopyStream(&input) && input.GetErrno() == 0;
}

bool Message::ParsePartialFromFileDescriptor(int file_descriptor) {
  io::FileInputStream input(file_descriptor);
  return ParsePartialFromZeroCopyStream(&input) && input.GetErrno() == 0;
}

bool Message::ParseFromIstream(std::istream* input) {
  io::IstreamInputStream zero_copy_input(input);
  return ParseFromZeroCopyStream(&zero_copy_input) && input->eof();
}

bool Message::ParsePartialFromIstream(std::istream* input) {
  io::IstreamInputStream zero_copy_input(input);
  return ParsePartialFromZeroCopyStream(&zero_copy_input) && input->eof();
}

#if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
namespace internal {

class ReflectionAccessor {
 public:
  static void* GetOffset(void* msg, const google::protobuf::FieldDescriptor* f,
                         const google::protobuf::Reflection* r) {
    auto gr =
        dynamic_cast<const google::protobuf::internal::GeneratedMessageReflection*>(r);
    GOOGLE_CHECK(gr != nullptr);
    return static_cast<char*>(msg) + gr->schema_.GetFieldOffset(f);
  }

  static google::protobuf::internal::ExtensionSet* GetExtensionSet(
      void* msg, const google::protobuf::Reflection* r) {
    auto gr =
        dynamic_cast<const google::protobuf::internal::GeneratedMessageReflection*>(r);
    GOOGLE_CHECK(gr != nullptr);
    return reinterpret_cast<google::protobuf::internal::ExtensionSet*>(
        static_cast<char*>(msg) + gr->schema_.GetExtensionSetOffset());
  }
  static google::protobuf::internal::InternalMetadataWithArena* GetMetadata(
      void* msg, const google::protobuf::Reflection* r) {
    auto gr =
        dynamic_cast<const google::protobuf::internal::GeneratedMessageReflection*>(r);
    GOOGLE_CHECK(gr != nullptr);
    return reinterpret_cast<google::protobuf::internal::InternalMetadataWithArena*>(
        static_cast<char*>(msg) + gr->schema_.GetMetadataOffset());
  }
};

}  // namespace internal

void SetField(uint64 val, const FieldDescriptor* field, Message* msg,
              const Reflection* reflection) {
#define STORE_TYPE(CPPTYPE_METHOD)                        \
  do                                                      \
    if (field->is_repeated()) {                           \
      reflection->Add##CPPTYPE_METHOD(msg, field, value); \
    } else {                                              \
      reflection->Set##CPPTYPE_METHOD(msg, field, value); \
    }                                                     \
  while (0)

  switch (field->type()) {
#define HANDLE_TYPE(TYPE, CPPTYPE, CPPTYPE_METHOD) \
  case FieldDescriptor::TYPE_##TYPE: {             \
    CPPTYPE value = val;                           \
    STORE_TYPE(CPPTYPE_METHOD);                    \
    break;                                         \
  }

    // Varints
    HANDLE_TYPE(INT32, int32, Int32)
    HANDLE_TYPE(INT64, int64, Int64)
    HANDLE_TYPE(UINT32, uint32, UInt32)
    HANDLE_TYPE(UINT64, uint64, UInt64)
    case FieldDescriptor::TYPE_SINT32: {
      int32 value = WireFormatLite::ZigZagDecode32(val);
      STORE_TYPE(Int32);
      break;
    }
    case FieldDescriptor::TYPE_SINT64: {
      int64 value = WireFormatLite::ZigZagDecode64(val);
      STORE_TYPE(Int64);
      break;
    }
      HANDLE_TYPE(BOOL, bool, Bool)

      // Fixed
      HANDLE_TYPE(FIXED32, uint32, UInt32)
      HANDLE_TYPE(FIXED64, uint64, UInt64)
      HANDLE_TYPE(SFIXED32, int32, Int32)
      HANDLE_TYPE(SFIXED64, int64, Int64)

    case FieldDescriptor::TYPE_FLOAT: {
      float value;
      uint32 bit_rep = val;
      std::memcpy(&value, &bit_rep, sizeof(value));
      STORE_TYPE(Float);
      break;
    }
    case FieldDescriptor::TYPE_DOUBLE: {
      double value;
      uint64 bit_rep = val;
      std::memcpy(&value, &bit_rep, sizeof(value));
      STORE_TYPE(Double);
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      int value = val;
      if (field->is_repeated()) {
        reflection->AddEnumValue(msg, field, value);
      } else {
        reflection->SetEnumValue(msg, field, value);
      }
      break;
    }
    default:
      GOOGLE_LOG(FATAL) << "Error in descriptors, primitve field with field type "
                 << field->type();
  }
#undef STORE_TYPE
#undef HANDLE_TYPE
}

bool ReflectiveValidator(const void* arg, int val) {
  auto d = static_cast<const EnumDescriptor*>(arg);
  return d->FindValueByNumber(val) != nullptr;
}

ParseClosure GetPackedField(const FieldDescriptor* field, Message* msg,
                            const Reflection* reflection,
                            internal::ParseContext* ctx) {
  switch (field->type()) {
#define HANDLE_PACKED_TYPE(TYPE, CPPTYPE, METHOD_NAME) \
  case FieldDescriptor::TYPE_##TYPE:                   \
    return {internal::Packed##METHOD_NAME##Parser,     \
            reflection->MutableRepeatedField<CPPTYPE>(msg, field)}
    HANDLE_PACKED_TYPE(INT32, int32, Int32);
    HANDLE_PACKED_TYPE(INT64, int64, Int64);
    HANDLE_PACKED_TYPE(SINT32, int32, SInt32);
    HANDLE_PACKED_TYPE(SINT64, int64, SInt64);
    HANDLE_PACKED_TYPE(UINT32, uint32, UInt32);
    HANDLE_PACKED_TYPE(UINT64, uint64, UInt64);
    HANDLE_PACKED_TYPE(BOOL, bool, Bool);
    case FieldDescriptor::TYPE_ENUM: {
      if (field->file()->syntax() == FileDescriptor::SYNTAX_PROTO3) {
        auto object =
            internal::ReflectionAccessor::GetOffset(msg, field, reflection);
        return {internal::PackedEnumParser, object};
      } else {
        ctx->extra_parse_data().SetEnumValidatorArg(
            ReflectiveValidator, field->enum_type(),
            reflection->MutableUnknownFields(msg), field->number());
        auto object =
            internal::ReflectionAccessor::GetOffset(msg, field, reflection);
        return {internal::PackedValidEnumParserArg, object};
      }
    }
      HANDLE_PACKED_TYPE(FIXED32, uint32, Fixed32);
      HANDLE_PACKED_TYPE(FIXED64, uint64, Fixed64);
      HANDLE_PACKED_TYPE(SFIXED32, int32, SFixed32);
      HANDLE_PACKED_TYPE(SFIXED64, int64, SFixed64);
      HANDLE_PACKED_TYPE(FLOAT, float, Float);
      HANDLE_PACKED_TYPE(DOUBLE, double, Double);
#undef HANDLE_PACKED_TYPE

    default:
      GOOGLE_LOG(FATAL) << "Type is not packable " << field->type();
  }
}

ParseClosure GetLenDelim(int field_number, const FieldDescriptor* field,
                         Message* msg, UnknownFieldSet* unknown,
                         const Reflection* reflection,
                         internal::ParseContext* ctx) {
  if (field == nullptr || WireFormat::WireTypeForFieldType(field->type()) !=
                              WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    if (field && field->is_packable()) {
      return GetPackedField(field, msg, reflection, ctx);
    }
    return {internal::StringParser, unknown->AddLengthDelimited(field_number)};
  }
  enum { kNone = 0, kVerify, kStrict } utf8_level = kNone;
  internal::ParseFunc string_parsers[] = {internal::StringParser,
                                          internal::StringParserUTF8Verify,
                                          internal::StringParserUTF8};
  internal::ParseFunc cord_parsers[] = {internal::CordParser,
                                        internal::CordParserUTF8Verify,
                                        internal::CordParserUTF8};
  internal::ParseFunc string_piece_parsers[] = {
      internal::StringPieceParser, internal::StringPieceParserUTF8Verify,
      internal::StringPieceParserUTF8};
  switch (field->type()) {
    case FieldDescriptor::TYPE_STRING:
      if (field->file()->syntax() == FileDescriptor::SYNTAX_PROTO3
      ) {
        ctx->extra_parse_data().SetFieldName(field->full_name().c_str());
        utf8_level = kStrict;
      } else if (
          true) {
        ctx->extra_parse_data().SetFieldName(field->full_name().c_str());
        utf8_level = kVerify;
      }
      GOOGLE_FALLTHROUGH_INTENDED;
    case FieldDescriptor::TYPE_BYTES: {
      if (field->is_repeated()) {
        int index = reflection->FieldSize(*msg, field);
        // Add new empty value.
        reflection->AddString(msg, field, "");
        if (field->options().ctype() == FieldOptions::STRING ||
            field->is_extension()) {
          auto object = reflection->MutableRepeatedPtrField<string>(msg, field)
                            ->Mutable(index);
          return {string_parsers[utf8_level], object};
        } else if (field->options().ctype() == FieldOptions::CORD) {
          auto object = reflection->MutableRepeatedField<Cord>(msg, field)
                            ->Mutable(index);
          return {cord_parsers[utf8_level], object};
        } else if (field->options().ctype() == FieldOptions::STRING_PIECE) {
          auto object =
              reflection
                  ->MutableRepeatedPtrField<internal::StringPieceField>(msg,
                                                                        field)
                  ->Mutable(index);
          return {string_piece_parsers[utf8_level], object};
        }
      } else {
        // Clear value and make sure it's set.
        reflection->SetString(msg, field, "");
        if (field->options().ctype() == FieldOptions::STRING ||
            field->is_extension()) {
          // HACK around inability to get mutable_string in reflection
          string* object = &const_cast<string&>(
              reflection->GetStringReference(*msg, field, nullptr));
          return {string_parsers[utf8_level], object};
        } else {
          void* object =
              internal::ReflectionAccessor::GetOffset(msg, field, reflection);
          if (field->containing_oneof()) {
            object = *static_cast<Cord**>(object);
          }
          if (field->options().ctype() == FieldOptions::CORD) {
            return {cord_parsers[utf8_level], object};
          } else if (field->options().ctype() == FieldOptions::STRING_PIECE) {
            return {string_piece_parsers[utf8_level], object};
          }
        }
      }
      GOOGLE_LOG(FATAL) << "No other type than string supported";
    }
    case FieldDescriptor::TYPE_MESSAGE: {
      Message* object;
      if (field->is_repeated()) {
        object = reflection->AddMessage(msg, field, nullptr);
      } else {
        object = reflection->MutableMessage(msg, field, nullptr);
      }
      return {object->_ParseFunc(), object};
    }
    default:
      GOOGLE_LOG(FATAL) << "Wrong type for length delim " << field->type();
  }
}

const char* ReflectiveParseMessageSetItem(const char* begin, const char* end,
                                          void* object,
                                          internal::ParseContext* ctx) {
  ParseClosure child;
  auto msg = static_cast<Message*>(object);
  auto reflection = msg->GetReflection();
  uint32 size;
  auto ptr = begin;
  while (ptr < end) {
    uint32 tag = *ptr++;
    if (tag == WireFormatLite::kMessageSetTypeIdTag) {
      uint32 type_id;
      ptr = Varint::Parse32(ptr, &type_id);
      if (!ptr) goto error;

      auto field = reflection->FindKnownExtensionByNumber(type_id);

      if (ctx->extra_parse_data().payload.empty()) {
        tag = *ptr++;
        if (tag == WireFormatLite::kMessageSetMessageTag) {
          ptr = Varint::Parse32Inline(ptr, &size);
          if (!ptr) goto error;
          child = GetLenDelim(type_id * 8 + 2, field, msg,
                              reflection->MutableUnknownFields(msg), reflection,
                              ctx);
          if (size > end - ptr) goto len_delim_till_end;
          auto newend = ptr + size;
          if (!ctx->ParseExactRange(child, ptr, newend)) goto error;
          ptr = newend;
        } else {
          goto error;
        }
      } else {
        GOOGLE_LOG(FATAL) << "Wrong order";
      }
    } else if (tag == WireFormatLite::kMessageSetItemEndTag) {
      if (!ctx->ValidEndGroup(tag)) goto error;
      break;
    } else if (tag == WireFormatLite::kMessageSetMessageTag) {
      uint32 size;
      ptr = Varint::Parse32Inline(ptr, &size);
      if (!ptr) goto error;
      child = {internal::StringParser, &ctx->extra_parse_data().payload};
      if (size > end - ptr) goto len_delim_till_end;
      auto newend = ptr + size;
      if (!ctx->ParseExactRange(child, ptr, newend)) goto error;
      ptr = newend;
    } else {
      GOOGLE_LOG(FATAL) << "Unknown message set item tag";
    }
  }
  return ptr;
error:
  return nullptr;
len_delim_till_end:
  return ctx->StoreAndTailCall(ptr, end, {ReflectiveParseMessageSetItem, msg},
                               child, size);
}

ParseClosure GetGroup(int field_number, const FieldDescriptor* field,
                      Message* msg, UnknownFieldSet* unknown,
                      const Reflection* reflection) {
  if (field == nullptr && field_number == 1 &&
      msg->GetDescriptor()->options().message_set_wire_format()) {
    return {ReflectiveParseMessageSetItem, msg};
  }
  if (field == nullptr || WireFormat::WireTypeForFieldType(field->type()) !=
                              WireFormatLite::WIRETYPE_START_GROUP) {
    return {internal::UnknownGroupParse, unknown->AddGroup(field_number)};
  }

  Message* object;
  if (field->is_repeated()) {
    object = reflection->AddMessage(msg, field, nullptr);
  } else {
    object = reflection->MutableMessage(msg, field, nullptr);
  }
  return {object->_ParseFunc(), object};
}

const char* Message::_InternalParse(const char* begin, const char* end,
                                    void* object, internal::ParseContext* ctx) {
  auto msg = static_cast<Message*>(object);
  const Descriptor* descriptor = msg->GetDescriptor();
  const Reflection* reflection = msg->GetReflection();
  auto unknown = reflection->MutableUnknownFields(msg);
  GOOGLE_DCHECK(descriptor);
  GOOGLE_DCHECK(reflection);

  GOOGLE_DCHECK(begin <= end);
  uint32 size;
  internal::ParseFunc parser_till_end;
  uint32 tag;
  int depth;
  auto ptr = begin;
  while (ptr < end) {
    ptr = Varint::Parse32Inline(ptr, &tag);
    if (ptr == nullptr) return nullptr;
    if ((tag >> 3) == 0) return nullptr;
    const FieldDescriptor* field = nullptr;

    int field_number = WireFormatLite::GetTagFieldNumber(tag);
    field = descriptor->FindFieldByNumber(field_number);

    // If that failed, check if the field is an extension.
    if (field == nullptr && descriptor->IsExtensionNumber(field_number)) {
      field = reflection->FindKnownExtensionByNumber(field_number);
    }

    // if (field) GOOGLE_LOG(ERROR) << "Encountered field " << field->name();

    switch (tag & 7) {
      case 0: {
        uint64 val;
        ptr = Varint::Parse64(ptr, &val);
        if (!ptr) goto error;
        if (field == nullptr ||
            WireFormat::WireTypeForFieldType(field->type()) != 0) {
          unknown->AddVarint(field_number, val);
          break;
        }
        SetField(val, field, msg, reflection);
        break;
      }
      case 1: {
        uint64 val = UNALIGNED_LOAD64(ptr);
        ptr = ptr + 8;
        if (field == nullptr ||
            WireFormat::WireTypeForFieldType(field->type()) != 1) {
          unknown->AddFixed64(field_number, val);
          break;
        }
        SetField(val, field, msg, reflection);
        break;
      }
      case 2: {
        ptr = Varint::Parse32Inline(ptr, &size);
        if (!ptr) goto error;
        ParseClosure child =
            GetLenDelim(field_number, field, msg, unknown, reflection, ctx);
        parser_till_end = child.func;
        object = child.object;
        if (size > end - ptr) goto len_delim_till_end;
        auto newend = ptr + size;
        if (!ctx->ParseExactRange(child, ptr, newend)) goto error;
        ptr = newend;
        break;
      }
      case 3: {
        if (!ctx->PrepareGroup(tag, &depth)) goto error;

        ParseClosure child =
            GetGroup(field_number, field, msg, unknown, reflection);
        parser_till_end = child.func;
        object = child.object;

        if (ptr < end) ptr = child(ptr, end, ctx);
        if (!ptr) goto error;
        if (ctx->GroupContinues(depth)) goto group_continues;
        break;
      }
      case 4: {
        if (!ctx->ValidEndGroup(tag)) goto error;
        return ptr;
      }
      case 5: {
        uint32 val = UNALIGNED_LOAD32(ptr);
        ptr = ptr + 4;
        if (field == nullptr ||
            WireFormat::WireTypeForFieldType(field->type()) != 5) {
          unknown->AddFixed64(field_number, val);
          break;
        }
        SetField(val, field, msg, reflection);
        break;
      }
      default:
        goto error;
    }
  }
  return ptr;
error:
  return nullptr;
len_delim_till_end:
  // Length delimited field crosses end
  return ctx->StoreAndTailCall(ptr, end, {Message::_InternalParse, msg},
                               {parser_till_end, object}, size);
group_continues:
  GOOGLE_DCHECK(ptr >= end);
  // Group crossed end and must be continued. Either this a parse failure
  // or we need to resume on the next chunk and thus save the state.
  ctx->StoreGroup({Message::_InternalParse, msg}, {parser_till_end, object},
                  depth);
  return ptr;
}
#endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER


void Message::SerializeWithCachedSizes(
    io::CodedOutputStream* output) const {
  const internal::SerializationTable* table =
      static_cast<const internal::SerializationTable*>(InternalGetTable());
  if (table == 0) {
    WireFormat::SerializeWithCachedSizes(*this, GetCachedSize(), output);
  } else {
    internal::TableSerialize(*this, table, output);
  }
}

size_t Message::ByteSizeLong() const {
  size_t size = WireFormat::ByteSize(*this);
  SetCachedSize(internal::ToCachedSize(size));
  return size;
}

void Message::SetCachedSize(int /* size */) const {
  GOOGLE_LOG(FATAL) << "Message class \"" << GetDescriptor()->full_name()
             << "\" implements neither SetCachedSize() nor ByteSize().  "
                "Must implement one or the other.";
}

size_t Message::SpaceUsedLong() const {
  return GetReflection()->SpaceUsedLong(*this);
}

bool Message::SerializeToFileDescriptor(int file_descriptor) const {
  io::FileOutputStream output(file_descriptor);
  return SerializeToZeroCopyStream(&output) && output.Flush();
}

bool Message::SerializePartialToFileDescriptor(int file_descriptor) const {
  io::FileOutputStream output(file_descriptor);
  return SerializePartialToZeroCopyStream(&output) && output.Flush();
}

bool Message::SerializeToOstream(std::ostream* output) const {
  {
    io::OstreamOutputStream zero_copy_output(output);
    if (!SerializeToZeroCopyStream(&zero_copy_output)) return false;
  }
  return output->good();
}

bool Message::SerializePartialToOstream(std::ostream* output) const {
  io::OstreamOutputStream zero_copy_output(output);
  return SerializePartialToZeroCopyStream(&zero_copy_output);
}


// =============================================================================
// Reflection and associated Template Specializations

Reflection::~Reflection() {}

void Reflection::AddAllocatedMessage(Message* /* message */,
                                     const FieldDescriptor* /*field */,
                                     Message* /* new_entry */) const {}

#define HANDLE_TYPE(TYPE, CPPTYPE, CTYPE)                             \
template<>                                                            \
const RepeatedField<TYPE>& Reflection::GetRepeatedField<TYPE>(        \
    const Message& message, const FieldDescriptor* field) const {     \
  return *static_cast<RepeatedField<TYPE>* >(                         \
      MutableRawRepeatedField(const_cast<Message*>(&message),         \
                          field, CPPTYPE, CTYPE, NULL));              \
}                                                                     \
                                                                      \
template<>                                                            \
RepeatedField<TYPE>* Reflection::MutableRepeatedField<TYPE>(          \
    Message* message, const FieldDescriptor* field) const {           \
  return static_cast<RepeatedField<TYPE>* >(                          \
      MutableRawRepeatedField(message, field, CPPTYPE, CTYPE, NULL)); \
}

HANDLE_TYPE(int32,  FieldDescriptor::CPPTYPE_INT32,  -1);
HANDLE_TYPE(int64,  FieldDescriptor::CPPTYPE_INT64,  -1);
HANDLE_TYPE(uint32, FieldDescriptor::CPPTYPE_UINT32, -1);
HANDLE_TYPE(uint64, FieldDescriptor::CPPTYPE_UINT64, -1);
HANDLE_TYPE(float,  FieldDescriptor::CPPTYPE_FLOAT,  -1);
HANDLE_TYPE(double, FieldDescriptor::CPPTYPE_DOUBLE, -1);
HANDLE_TYPE(bool,   FieldDescriptor::CPPTYPE_BOOL,   -1);


#undef HANDLE_TYPE

void* Reflection::MutableRawRepeatedString(
    Message* message, const FieldDescriptor* field, bool is_string) const {
  return MutableRawRepeatedField(message, field,
      FieldDescriptor::CPPTYPE_STRING, FieldOptions::STRING, NULL);
}


MapIterator Reflection::MapBegin(
    Message* message,
    const FieldDescriptor* field) const {
  GOOGLE_LOG(FATAL) << "Unimplemented Map Reflection API.";
  MapIterator iter(message, field);
  return iter;
}

MapIterator Reflection::MapEnd(
    Message* message,
    const FieldDescriptor* field) const {
  GOOGLE_LOG(FATAL) << "Unimplemented Map Reflection API.";
  MapIterator iter(message, field);
  return iter;
}

// =============================================================================
// MessageFactory

MessageFactory::~MessageFactory() {}

namespace internal {

// TODO(gerbens) make this factorized better. This should not have to hop
// to reflection. Currently uses GeneratedMessageReflection and thus is
// defined in generated_message_reflection.cc
void RegisterFileLevelMetadata(void* assign_descriptors_table);

}  // namespace internal

namespace {

void RegisterFileLevelMetadata(void* assign_descriptors_table,
                               const string& filename) {
  internal::RegisterFileLevelMetadata(assign_descriptors_table);
}

class GeneratedMessageFactory : public MessageFactory {
 public:
  static GeneratedMessageFactory* singleton();

  struct RegistrationData {
    const Metadata* file_level_metadata;
    int size;
  };

  void RegisterFile(const char* file, void* registration_data);
  void RegisterType(const Descriptor* descriptor, const Message* prototype);

  // implements MessageFactory ---------------------------------------
  const Message* GetPrototype(const Descriptor* type) override;

 private:
  // Only written at static init time, so does not require locking.
  std::unordered_map<const char*, void*, hash<const char*>,
                     streq>
      file_map_;

  internal::WrappedMutex mutex_;
  // Initialized lazily, so requires locking.
  std::unordered_map<const Descriptor*, const Message*> type_map_;
};

GeneratedMessageFactory* GeneratedMessageFactory::singleton() {
  static auto instance =
      internal::OnShutdownDelete(new GeneratedMessageFactory);
  return instance;
}

void GeneratedMessageFactory::RegisterFile(const char* file,
                                           void* registration_data) {
  if (!InsertIfNotPresent(&file_map_, file, registration_data)) {
    GOOGLE_LOG(FATAL) << "File is already registered: " << file;
  }
}

void GeneratedMessageFactory::RegisterType(const Descriptor* descriptor,
                                           const Message* prototype) {
  GOOGLE_DCHECK_EQ(descriptor->file()->pool(), DescriptorPool::generated_pool())
    << "Tried to register a non-generated type with the generated "
       "type registry.";

  // This should only be called as a result of calling a file registration
  // function during GetPrototype(), in which case we already have locked
  // the mutex.
  mutex_.AssertHeld();
  if (!InsertIfNotPresent(&type_map_, descriptor, prototype)) {
    GOOGLE_LOG(DFATAL) << "Type is already registered: " << descriptor->full_name();
  }
}


const Message* GeneratedMessageFactory::GetPrototype(const Descriptor* type) {
  {
    ReaderMutexLock lock(&mutex_);
    const Message* result = FindPtrOrNull(type_map_, type);
    if (result != NULL) return result;
  }

  // If the type is not in the generated pool, then we can't possibly handle
  // it.
  if (type->file()->pool() != DescriptorPool::generated_pool()) return NULL;

  // Apparently the file hasn't been registered yet.  Let's do that now.
  void* registration_data =
      FindPtrOrNull(file_map_, type->file()->name().c_str());
  if (registration_data == NULL) {
    GOOGLE_LOG(DFATAL) << "File appears to be in generated pool but wasn't "
                   "registered: " << type->file()->name();
    return NULL;
  }

  WriterMutexLock lock(&mutex_);

  // Check if another thread preempted us.
  const Message* result = FindPtrOrNull(type_map_, type);
  if (result == NULL) {
    // Nope.  OK, register everything.
    RegisterFileLevelMetadata(registration_data, type->file()->name());
    // Should be here now.
    result = FindPtrOrNull(type_map_, type);
  }

  if (result == NULL) {
    GOOGLE_LOG(DFATAL) << "Type appears to be in generated pool but wasn't "
                << "registered: " << type->full_name();
  }

  return result;
}

}  // namespace

MessageFactory* MessageFactory::generated_factory() {
  return GeneratedMessageFactory::singleton();
}

void MessageFactory::InternalRegisterGeneratedFile(
    const char* filename, void* assign_descriptors_table) {
  GeneratedMessageFactory::singleton()->RegisterFile(filename,
                                                     assign_descriptors_table);
}

void MessageFactory::InternalRegisterGeneratedMessage(
    const Descriptor* descriptor, const Message* prototype) {
  GeneratedMessageFactory::singleton()->RegisterType(descriptor, prototype);
}


MessageFactory* Reflection::GetMessageFactory() const {
  GOOGLE_LOG(FATAL) << "Not implemented.";
  return NULL;
}

void* Reflection::RepeatedFieldData(
    Message* message, const FieldDescriptor* field,
    FieldDescriptor::CppType cpp_type,
    const Descriptor* message_type) const {
  GOOGLE_LOG(FATAL) << "Not implemented.";
  return NULL;
}

namespace {
template <typename T>
T* GetSingleton() {
  static T singleton;
  return &singleton;
}
}  // namespace

const internal::RepeatedFieldAccessor* Reflection::RepeatedFieldAccessor(
    const FieldDescriptor* field) const {
  GOOGLE_CHECK(field->is_repeated());
  switch (field->cpp_type()) {
#define HANDLE_PRIMITIVE_TYPE(TYPE, type) \
  case FieldDescriptor::CPPTYPE_##TYPE:   \
    return GetSingleton<internal::RepeatedFieldPrimitiveAccessor<type> >();
    HANDLE_PRIMITIVE_TYPE(INT32, int32)
    HANDLE_PRIMITIVE_TYPE(UINT32, uint32)
    HANDLE_PRIMITIVE_TYPE(INT64, int64)
    HANDLE_PRIMITIVE_TYPE(UINT64, uint64)
    HANDLE_PRIMITIVE_TYPE(FLOAT, float)
    HANDLE_PRIMITIVE_TYPE(DOUBLE, double)
    HANDLE_PRIMITIVE_TYPE(BOOL, bool)
    HANDLE_PRIMITIVE_TYPE(ENUM, int32)
#undef HANDLE_PRIMITIVE_TYPE
    case FieldDescriptor::CPPTYPE_STRING:
      switch (field->options().ctype()) {
        default:
        case FieldOptions::STRING:
          return GetSingleton<internal::RepeatedPtrFieldStringAccessor>();
      }
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      if (field->is_map()) {
        return GetSingleton<internal::MapFieldAccessor>();
      } else {
        return GetSingleton<internal::RepeatedPtrFieldMessageAccessor>();
      }
  }
  GOOGLE_LOG(FATAL) << "Should not reach here.";
  return NULL;
}

namespace internal {
template <>
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
// Note: force noinline to workaround MSVC compiler bug with /Zc:inline, issue
// #240
GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE
#endif
    Message*
    GenericTypeHandler<Message>::NewFromPrototype(const Message* prototype,
                                                  Arena* arena) {
  return prototype->New(arena);
}
template <>
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
// Note: force noinline to workaround MSVC compiler bug with /Zc:inline, issue
// #240
GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE
#endif
    Arena*
    GenericTypeHandler<Message>::GetArena(Message* value) {
  return value->GetArena();
}
template <>
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
// Note: force noinline to workaround MSVC compiler bug with /Zc:inline, issue
// #240
GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE
#endif
    void*
    GenericTypeHandler<Message>::GetMaybeArenaPointer(Message* value) {
  return value->GetMaybeArenaPointer();
}
}  // namespace internal

}  // namespace protobuf
}  // namespace google
