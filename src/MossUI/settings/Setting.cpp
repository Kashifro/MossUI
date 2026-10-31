#include "MossUI/settings/Setting.hpp"

namespace MossUI {

namespace {
Store* store = nullptr;
}

void SetStore(Store* s) { store = s; }
Store* CurrentStore() { return store; }

void Setting::Reset() const {
    struct Visitor {
        const Setting& self;
        void operator()(const row::Toggle& r) const { if (r.set) r.set(r.def); }
        void operator()(const row::Key& r) const { if (r.set) r.set(r.def); }
        void operator()(const row::Slider& r) const { if (r.set) r.set(r.def); }
        void operator()(const row::Color& r) const { if (r.set) r.set(r.def); }
        void operator()(const row::Choice& r) const { if (r.set) r.set(r.def); }
        void operator()(const row::Text& r) const { if (r.set) r.set(r.def); }
        void operator()(const row::Action&) const {}
        void operator()(const row::Section&) const {}
        void operator()(const row::Category&) const { for (const auto& c : self.children) c.Reset(); }
    };
    std::visit(Visitor{*this}, data);
}

}
