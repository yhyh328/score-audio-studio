#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <node.h>
#include <string>
#include <v8.h>

#include "score_audio_studio/dsp/PlaybackEvent.hpp"

namespace {

using PlaybackEvent = score_audio_studio::dsp::PlaybackEvent;

void consume(
    const v8::FunctionCallbackInfo<v8::Value>& args
)
{
    v8::Isolate* isolate = args.GetIsolate();
    if (
        args.Length() != 2
        ||
        !args[0]->IsSharedArrayBuffer()
        ||
        !args[1]->IsUint32()
    ) {
        isolate->ThrowException(
            v8::Exception::TypeError(
                v8::String::NewFromUtf8Literal(
                    isolate,
                    "consume expects SharedArrayBuffer and count"
                )
            )
        );
        return;
    }
    const v8::Local<v8::SharedArrayBuffer> buffer =
        args[0].As<v8::SharedArrayBuffer>();

    const std::shared_ptr<v8::BackingStore> backingStore =
        buffer->GetBackingStore();

    const std::uint32_t count =
        args[1].As<v8::Uint32>()->Value();

    const std::size_t requiredBytes =
        static_cast<std::size_t>(count) * sizeof(PlaybackEvent);

    if (
        count == 0
        ||
        backingStore->ByteLength() < requiredBytes
    ) {
        isolate->ThrowException(
            v8::Exception::RangeError(
                v8::String::NewFromUtf8Literal(
                    isolate,
                    "Invalid event count or buffer size"
                )
            )
        );
        return;
    }

    const void* data = backingStore->Data();
    PlaybackEvent first{};
    std::memcpy(&first, data, sizeof(first));

#ifndef NDEBUG
    std::cout
        << "address="
        << data << '\n'
        << "count="
        << count << '\n'
        << "tick="
        << first.sampleOffset << '\n'
        << "instrumentIndex="
        << first.partId << '\n'
        << "type="
        << static_cast<unsigned>(first.type) << '\n'
        << "midiNoteNumber="
        << static_cast<unsigned>(first.midiNoteNumber) << '\n'
        << "velocity="
        << first.velocity << '\n'
        << "noteId="
        << first.noteId << '\n';
#endif

    args.GetReturnValue().Set(
        v8::Integer::NewFromUnsigned(
            isolate,
            count
        )
    );
}

}  // namespace

NODE_MODULE_INIT()
{
    (void)module;
    (void)context;
    NODE_SET_METHOD(exports, "consume", consume);
}
