#include <string>
#include <vector>

#include "dream/translator/emit.h"

#include "doctest.h"

using dream::translator::emit_unit;
using dream::translator::EmitOptions;
using dream::translator::FunctionSpec;
using dream::translator::Image;

// The entry interrupt poll runs before the resume switch. On a resumed entry the guest is at
// resume_pc, so that is what SPC must record: with the function entry instead, a task switch taken
// at this poll came back to the entry and re-ran the prologue on the resumed frame's stack
// (Soulcalibur, docs/emitter-design.md "Non-local returns").
TEST_CASE("emit: the entry poll records resume_pc on a resumed entry") {
    Image image;
    image.base = 0x8C010000u;
    // mov #1,r0 ; rts ; nop
    image.bytes = {0x01, 0xE0, 0x0B, 0x00, 0x09, 0x00};
    FunctionSpec f;
    f.entry = 0x8C010000u;
    f.end = 0x8C010006u;
    const auto r = emit_unit(image, {f}, EmitOptions{});
    CHECK(r.source.find("c.pc = resume_pc ? resume_pc : 0x8c010000u; deliver_irq(c, m);") !=
          std::string::npos);
    CHECK(r.source.find("c.pc = 0x8c010000u; deliver_irq(c, m);") == std::string::npos);
}

// A valid instruction with no lowering fails the translation (counted in `unsupported`); a word
// that decodes to no instruction is data discovery walked into and stays a runtime fault.
TEST_CASE("emit: valid unlowered instructions are unsupported, invalid words are not") {
    Image image;
    image.base = 0x8C010000u;
    // sleep ; .word 0x0000 ; rts ; nop
    image.bytes = {0x1B, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x09, 0x00};
    FunctionSpec f;
    f.entry = 0x8C010000u;
    f.end = 0x8C010008u;
    const auto r = emit_unit(image, {f}, EmitOptions{});
    CHECK(r.unlowered == 2);
    CHECK(r.unsupported == 1);
}

// The table row's end is what the body emitted, not the wider extent discovery claimed, so a
// resume or interior jump past it looks for a host function that has the label (Hydro Thunder
// 0x0c1cc46c: a fragment ending at an unrecovered braf hid its host's switch cases).
TEST_CASE("emit: the function table ends at the last emitted instruction") {
    Image image;
    image.base = 0x8C010000u;
    // rts ; nop ; mov #1,r0 ; rts ; nop  -- the second half is never reached from the entry
    image.bytes = {0x0B, 0x00, 0x09, 0x00, 0x01, 0xE0, 0x0B, 0x00, 0x09, 0x00};
    FunctionSpec f;
    f.entry = 0x8C010000u;
    f.end = 0x8C01000Au;
    f.name = "fn_frag";
    const auto r = emit_unit(image, {f}, EmitOptions{});
    CHECK(r.source.find("{0x8c010000u, &fn_frag, nullptr, 0, 0x8c010004u, &fn_frag__resume}") !=
          std::string::npos);
}

// Generated code reaches the runtime through one versioned header and nothing else.
TEST_CASE("emit: units include only gen_interface.h and check its version") {
    Image image;
    image.base = 0x8C010000u;
    image.bytes = {0x01, 0xE0, 0x0B, 0x00, 0x09, 0x00};
    FunctionSpec f;
    f.entry = 0x8C010000u;
    f.end = 0x8C010006u;
    const auto r = emit_unit(image, {f}, EmitOptions{});
    for (const std::string* text : {&r.source, &r.header}) {
        CHECK(text->find("#include \"dream/runtime/gen_interface.h\"") != std::string::npos);
        CHECK(text->find("static_assert(DREAM_GEN_INTERFACE_VERSION == ") != std::string::npos);
        const auto first = text->find("#include \"dream/");
        CHECK(text->find("#include \"dream/", first + 1) == std::string::npos);
    }
}
