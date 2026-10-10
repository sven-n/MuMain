#!/usr/bin/env python3
"""Regression fixtures for the build's RML contract guard."""
import pathlib
import contextlib
import io
import shutil
import subprocess
import sys
import unittest
import uuid


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.dont_write_bytecode = True
sys.path.insert(0, str(REPO_ROOT / "tools"))
from check_rml_rcss_drift import check_contracts


class RmlContractTests(unittest.TestCase):
    def setUp(self):
        self.root = REPO_ROOT / (".rml-contract-test-" + uuid.uuid4().hex)
        self.root.resolve().relative_to(REPO_ROOT.resolve())
        self.root.mkdir()
        self.addCleanup(shutil.rmtree, self.root)
        self.source = self.root / "source"
        self.assets = self.root / "assets"
        self.source.mkdir()
        self.assets.mkdir()
        for theme in ("legacy", "modern"):
            (self.assets / "themes" / theme).mkdir(parents=True)

    def write(self, path, text):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def window(self, documents=("panel",), bindings=None, body=None):
        paths = ", ".join('{"Data/Interface/RmlUi/' + name + '.rml"}' for name in documents)
        self.write(self.source / "Window.h", """
            class Window {
                UI::RmlBridge::ThemedView<PanelModel> view{"panel",
                    [this](auto& c, auto& model) { BindModel(c, model); },
                    {""" + paths + """}};
            };
        """)
        bindings = bindings if bindings is not None else 'c.Bind("value", &model.value);'
        body = body if body is not None else 'view.Document()->GetElementById("button");'
        self.write(self.source / "Window.cpp", """
            void Window::BindModel(Rml::DataModelConstructor& c, PanelModel& model) {
                """ + bindings + """
            }
            void Window::Update() { """ + body + """ }
        """)

    def markup(self, document="panel", text='<div id="button">{{value}}</div>', theme=None):
        directory = self.assets / "themes" / theme if theme else self.assets
        self.write(directory / (document + ".rml"), "<rml>" + text + "</rml>")

    def check(self):
        return check_contracts(self.source, self.assets)

    def assert_clean(self):
        errors, documents, variants, files = self.check()
        self.assertEqual(errors, [])
        self.assertGreater(documents, 0)
        self.assertEqual(variants, documents * 2)

    def test_header_view_and_shared_fallback_are_checked(self):
        self.window()
        self.markup()
        self.assert_clean()
        self.assertEqual(self.check()[1:], (1, 2, 1))

    def test_missing_id_in_one_fork_fails_even_when_other_theme_has_it(self):
        self.window()
        self.markup()
        self.markup(text='<!-- id="button" --><div id="renamed">{{value}}</div>', theme="modern")
        self.assertIn("modern/panel: missing required id 'button'", self.check()[0])

    def test_missing_callback_in_one_fork_fails(self):
        self.window(bindings='c.BindEventCallback("press", [](auto, auto&, auto&) {});')
        self.markup(text='<div id="button" data-event-click="press"></div>')
        self.markup(text='<div id="button"></div>', theme="modern")
        self.assertIn("modern/(panel): missing required callback 'press'", self.check()[0])

    def test_fields_can_have_alternative_theme_readouts(self):
        self.window(bindings='c.Bind("full", &model.full); c.Bind("current", &model.current);')
        self.markup(text='<div id="button">{{full}}</div>')
        self.markup(text='<div id="button">{{current}}</div>', theme="modern")
        self.assert_clean()

    def test_field_mentions_in_comments_and_strings_do_not_count(self):
        self.window()
        self.markup(text='<div id="button" data-if="other == \'value\'"></div><!-- {{value}} -->')
        self.assertTrue(any("field 'value'" in error for error in self.check()[0]))

    def test_commented_template_link_is_ignored(self):
        self.window()
        self.markup(text='<!-- <link type="text/template" href="missing.rml"/> --><div id="button">{{value}}</div>')
        self.assert_clean()

    def test_struct_member_does_not_satisfy_a_root_field(self):
        self.window()
        self.markup(text='<div id="button">{{line.value}}</div>')
        self.assertTrue(any("field 'value'" in error for error in self.check()[0]))

    def test_multiple_documents_scope_ids_and_share_callbacks(self):
        self.window(documents=("panel", "top"),
                    bindings='c.BindEventCallback("press", [](auto, auto&, auto&) {});',
                    body='auto* top = view.Document(1); top->GetElementById("top_button");')
        self.markup(text='<div data-event-click="press"></div>')
        self.markup("top", '<div id="top_button"></div>')
        self.assert_clean()
        self.markup("top", '<div></div>', theme="modern")
        self.assertIn("modern/top: missing required id 'top_button'", self.check()[0])

    def test_two_classes_in_one_header_keep_their_models_separate(self):
        self.write(self.source / "Windows.h", """
            class A { using Model = Alpha;
                UI::RmlBridge::ThemedView<Model> view{"a", Bind, {{"Data/Interface/RmlUi/a.rml"}}}; };
            class B { using Model = Beta;
                UI::RmlBridge::ThemedView<Model> view{"b", Bind, {{"Data/Interface/RmlUi/b.rml"}}}; };
        """)
        self.write(self.source / "Windows.cpp", """
            void A::Bind(Rml::DataModelConstructor& c, Alpha& model) { c.Bind("alpha", &model.value); }
            void B::Bind(Rml::DataModelConstructor& c, Beta& model) { c.Bind("beta", &model.value); }
        """)
        self.markup("a", "{{alpha}}")
        self.markup("b", "{{beta}}")
        self.assert_clean()

    def test_linked_templates_resolve_per_theme_and_transitively(self):
        self.window()
        self.markup(text='<link type="text/template" href="parts/shell.rml"/>')
        for theme in ("legacy", "modern"):
            directory = self.assets / "themes" / theme / "parts"
            self.write(directory / "shell.rml", '<link type="text/template" href="content.rml"/>')
            self.write(directory / "content.rml", '<div id="button">{{value}}</div>')
        self.assert_clean()
        self.write(self.assets / "themes/modern/parts/content.rml", "{{value}}")
        self.assertIn("modern/panel: missing required id 'button'", self.check()[0])

    def test_missing_template_fails_with_a_diagnostic(self):
        self.window()
        self.markup(text='<link type="text/template" href="missing.rml"/>')
        with self.assertRaisesRegex(ValueError, "Missing document/template"):
            self.check()

    def test_template_cycle_does_not_loop(self):
        self.window()
        self.markup(text='<link type="text/template" href="shell.rml"/>')
        for theme in ("legacy", "modern"):
            directory = self.assets / "themes" / theme
            self.write(directory / "shell.rml", '<link type="text/template" href="shell.rml"/><div id="button">{{value}}</div>')
        self.assert_clean()

    def test_constructor_initialized_view(self):
        self.window()
        self.write(self.source / "Window.h", "class Window { UI::RmlBridge::ThemedView<PanelModel> view; };")
        implementation = (self.source / "Window.cpp").read_text()
        self.write(self.source / "Window.cpp", 'Window::Window() : view("panel", BindModel, {{"Data/Interface/RmlUi/panel.rml"}}) {}\n' + implementation)
        self.markup()
        self.assert_clean()

    def test_reusable_view_covers_its_literal_callers(self):
        self.write(self.source / "Entry.h", "class Entry { UI::RmlBridge::ThemedView<EntryModel> view; };")
        self.write(self.source / "Entry.cpp", """
            Entry::Entry(const char* model, const char* path) : view(model, Bind, {{path}}) {}
            void Entry::Bind(Rml::DataModelConstructor& c, EntryModel& model) { c.Bind("value", &model.value); }
            void Entry::Update() { view.Document()->GetElementById("button"); }
        """)
        self.write(self.source / "Window.h", 'class Window { Entry entry{"one", "Data/Interface/RmlUi/one.rml"}; };')
        self.write(self.source / "Other.h", "class Other { Entry entry; };")
        self.write(self.source / "Other.cpp", 'Other::Other() : entry("two", "Data/Interface/RmlUi/two.rml") {}')
        self.markup("one")
        self.markup("two")
        self.assert_clean()

    def test_instanced_model_bind_method_is_discovered(self):
        self.write(self.source / "Room.h", """
            class Room { using Model = RoomModel;
                UI::RmlBridge::ThemedView<Model> view{"", [this](auto& c, auto& m) { Register(c, m); },
                    {{"Data/Interface/RmlUi/room.rml"}}, {.modelPlaceholder = "room"}}; };
        """)
        self.write(self.source / "Room.cpp", 'void Room::Register(Rml::DataModelConstructor& c, Model& m) { m.Bind(c); }')
        self.write(self.source / "RoomModel.cpp", 'void RoomModel::Bind(Rml::DataModelConstructor& c) { c.Bind("value", &value); }')
        self.markup("room", "{{value}}")
        self.assert_clean()

    def test_reusable_view_free_function_bindings_are_discovered(self):
        self.window()
        self.write(self.source / "Window.cpp", 'void Register(Rml::DataModelConstructor& c, PanelModel& model) { c.Bind("value", &model.value); }')
        self.markup(text='<div id="button"></div>')
        self.assertTrue(any("field 'value'" in error for error in self.check()[0]))

    def test_typed_model_with_undiscovered_bindings_fails(self):
        self.window(bindings='RegisterElsewhere(c, model);')
        self.markup()
        with self.assertRaisesRegex(ValueError, "no literal bindings discovered"):
            self.check()

    def test_direct_literal_load_is_discovered(self):
        self.write(self.source / "Loading.cpp", 'void Load() { auto* doc = context->LoadDocument("Data/Interface/RmlUi/loading.rml"); doc->GetElementById("progress"); }')
        self.markup("loading", '<div id="progress"></div>')
        self.assert_clean()
        self.markup("loading", "<div></div>", theme="modern")
        self.assertIn("modern/loading: missing required id 'progress'", self.check()[0])

    def test_aliased_make_unique_view_is_discovered(self):
        self.write(self.source / "Overlay.cpp", """
            using View = UI::RmlBridge::ThemedView<OverlayModel>;
            void Register(Rml::DataModelConstructor& c, OverlayModel& model) { c.Bind("value", &model.value); }
            void Show() { auto view = std::make_unique<View>("overlay", Register,
                std::vector<UI::RmlBridge::ThemedDocumentSpec>{{"Data/Interface/RmlUi/overlay.rml"}}); }
        """)
        self.markup("overlay", "{{value}}")
        self.assert_clean()

    def test_binding_free_companion_does_not_inherit_model_fields(self):
        self.window()
        header = (self.source / "Window.h").read_text()
        self.write(self.source / "Window.h", header.replace("class Window {", 'class Window { UI::RmlBridge::ThemedView<> background{{{"Data/Interface/RmlUi/background.rml"}}};'))
        self.markup()
        self.markup("background", "<div></div>")
        self.assert_clean()

    def test_unused_field_exceptions_are_scoped_and_reject_staleness(self):
        self.window(bindings='c.Bind("internal", &model.internal);')
        implementation = (self.source / "Window.cpp").read_text()
        self.write(self.source / "Window.cpp", "// rml-contract-unused: internal: used only by native state bookkeeping\n" + implementation)
        self.markup(text='<div id="button"></div>')
        self.assert_clean()
        self.markup(text='<div id="button">{{internal}}</div>')
        self.assertTrue(any("is stale" in error for error in self.check()[0]))

    def test_optional_markup_contract_needs_reason_and_rejects_staleness(self):
        self.window()
        self.markup(text='<!-- rml-contract-optional-id: button: this variant has no button -->{{value}}')
        self.assert_clean()
        self.markup(text='<!-- rml-contract-optional-id: button: this variant has no button --><div id="button">{{value}}</div>')
        self.assertTrue(any("stale optional-id" in error for error in self.check()[0]))
        self.markup(text='<!-- rml-contract-optional-id: button: -->{{value}}')
        with self.assertRaisesRegex(ValueError, "needs a reason"):
            self.check()

    def test_optional_callback_is_checked_independently_per_theme(self):
        self.window(bindings='c.BindEventCallback("press", [](auto, auto&, auto&) {});')
        self.markup(text='<div id="button" data-event-click="press"></div>')
        self.markup(text='<!-- rml-contract-optional-callback: press: variant submits with Enter --><div id="button"></div>', theme="modern")
        self.assert_clean()
        self.markup(text='<!-- rml-contract-optional-callback: press: variant submits with Enter --><div id="button" data-event-click="press"></div>', theme="modern")
        self.assertTrue(any("stale optional-callback" in error for error in self.check()[0]))

    def test_exception_from_an_unrelated_source_does_not_hide_drift(self):
        self.window()
        self.write(self.source / "Other.cpp", "// rml-contract-unused: value: unrelated model bookkeeping")
        self.markup(text='<div id="button"></div>')
        self.assertTrue(any("field 'value'" in error for error in self.check()[0]))

    def test_review_reports_exceptions_without_changing_coverage(self):
        self.window()
        self.markup(text='<!-- rml-contract-optional-id: button: no button in this variant -->{{value}}')
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = check_contracts(self.source, self.assets, review=True)
        self.assertEqual(result, self.check())
        self.assertIn("optional id button: no button in this variant", output.getvalue())

    def test_zero_coverage_fails_at_the_command_line(self):
        result = subprocess.run([sys.executable, "-B", str(REPO_ROOT / "tools/check_rml_rcss_drift.py"),
                                 "--source-root", str(self.source), "--asset-root", str(self.assets)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("zero coverage", result.stderr)

    def test_undiscovered_document_path_is_an_error(self):
        self.window()
        self.markup()
        self.write(self.source / "Unknown.cpp", 'const char* path = "Data/Interface/RmlUi/unknown.rml";')
        with self.assertRaisesRegex(ValueError, "Undiscovered document ownership"):
            self.check()

    def test_repository_contracts_have_nonzero_coverage(self):
        errors, documents, variants, files = check_contracts(REPO_ROOT / "src/source", REPO_ROOT / "src/bin/Data/Interface/RmlUi")
        self.assertEqual(errors, [])
        self.assertGreater(documents, 0)
        self.assertGreater(variants, documents)


if __name__ == "__main__":
    unittest.main()
