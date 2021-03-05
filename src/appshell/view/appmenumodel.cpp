//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "appmenumodel.h"

#include "translation.h"

#include "log.h"

#include "notation/internal/addmenucontroller.h"
#include "filemenucontroller.h"
#include "editmenucontroller.h"
#include "viewmenucontroller.h"

using namespace mu::appshell;
using namespace mu::uicomponents;
using namespace mu::notation;
using namespace mu::workspace;
using namespace mu::actions;

AppMenuModel::AppMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
    m_addMenuController = new AddMenuController();
    m_fileMenuController = new FileMenuController();
    m_editMenuController = new EditMenuController();
    m_viewMenuController = new ViewMenuController();
}

AppMenuModel::~AppMenuModel()
{
    delete m_addMenuController;
    delete m_fileMenuController;
    delete m_editMenuController;
    delete m_viewMenuController;
}

void AppMenuModel::load()
{
    TRACEFUNC;

    clear();

    appendItem(fileItem());
    appendItem(editItem());
    appendItem(viewItem());
    appendItem(addItem());
    appendItem(formatItem());
    appendItem(toolsItem());
    appendItem(helpItem());

    setupConnections();

    emit itemsChanged();
}

void AppMenuModel::handleAction(const QString& actionCodeStr, int actionIndex)
{
    ActionCode actionCode = codeFromQString(actionCodeStr);
    MenuItem menuItem = actionIndex == -1 ? findItem(actionCode) : findItemByIndex(actionCode, actionIndex);

    actionsDispatcher()->dispatch(actionCode, menuItem.args);
}

IMasterNotationPtr AppMenuModel::currentMasterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationPtr AppMenuModel::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr AppMenuModel::notationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr AppMenuModel::notationSelection() const
{
    return notationInteraction() ? notationInteraction()->selection() : nullptr;
}

void AppMenuModel::setupConnections()
{
    m_addMenuController->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        updateItemsEnabled(actionCodes);
    });

    m_fileMenuController->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        updateItemsEnabled(actionCodes);
    });

    m_editMenuController->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        updateItemsEnabled(actionCodes);
    });

    m_viewMenuController->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        updateItemsEnabled(actionCodes);
    });
        currentMasterNotation()->notation()->notationChanged().onNotify(this, [this]() {
            load();
        });

        currentMasterNotation()->notation()->interaction()->selectionChanged().onNotify(this, [this]() {
            load();
        });
    });

    userScoresService()->recentScoreList().ch.onReceive(this, [this](const MetaList&) {
        load();
    });

    paletteController()->isMasterPaletteOpened().ch.onReceive(this, [this](bool) {
        load();
    });

    notationPageState()->panelVisibleChanged().onReceive(this, [this](PanelType) {
        load();
    });

    controller()->isFullScreen().ch.onReceive(this, [this](bool) {
        load();
    });

    interactive()->currentUri().ch.onReceive(this, [this](const Uri&) {
        load();
    });
}

MenuItem AppMenuModel::fileItem()
{
    MenuItemList fileItems {
        makeAction("file-new", [this]() { return m_fileMenuController->isNewAvailable(); }),
        makeAction("file-open", [this]() { return m_fileMenuController->isOpenAvailable(); }),
        makeMenu(trc("appshell", "Open &Recent"), recentScores(), true, "file-open"),
        makeSeparator(),
        makeAction("file-close", [this]() { return m_fileMenuController->isCloseAvailable(); }),
        makeAction("file-save", [this]() { return m_fileMenuController->isSaveAvailable(SaveMode::Save); }),
        makeAction("file-save-as", [this]() { return m_fileMenuController->isSaveAvailable(SaveMode::SaveAs); }),
        makeAction("file-save-a-copy", [this]() { return m_fileMenuController->isSaveAvailable(SaveMode::SaveCopy); }),
        makeAction("file-save-selection", [this]() { return m_fileMenuController->isSaveAvailable(SaveMode::SaveSelection); }),
        makeAction("file-save-online", [this]() { return m_fileMenuController->isSaveAvailable(SaveMode::SaveOnline); }), // need implement
        makeSeparator(),
        makeAction("file-import-pdf", [this]() { return m_fileMenuController->isImportAvailable(); }),
        makeAction("file-export", [this]() { return m_fileMenuController->isExportAvailable(); }), // need implement
        makeSeparator(),
        makeAction("edit-info", [this]() { return m_fileMenuController->isEditInfoAvailable(); }),
        makeAction("parts", [this]() { return m_fileMenuController->isPartsAvailable(); }),
        makeSeparator(),
        makeAction("print", [this]() { return m_fileMenuController->isPrintAvailable(); }), // need implement
        makeSeparator(),
        makeAction("quit", [this]() { return m_fileMenuController->isQuitAvailable(); })
    };

    return makeMenu(trc("appshell", "&File"), fileItems);
}

MenuItem AppMenuModel::editItem()
{
    MenuItemList editItems {
        makeAction("undo", [this]() { return m_editMenuController->isUndoAvailable(); }),
        makeAction("redo", [this]() { return m_editMenuController->isRedoAvailable(); }),
        makeSeparator(),
        makeAction("cut", [this]() { return m_editMenuController->isCutAvailable(); }),
        makeAction("copy", [this]() { return m_editMenuController->isCopyAvailable(); }),
        makeAction("paste", [this]() { return m_editMenuController->isPasteAvailable(PastingType::Default); }),
        makeAction("paste-half", [this]() { return m_editMenuController->isPasteAvailable(PastingType::Half); }),
        makeAction("paste-double", [this]() { return m_editMenuController->isPasteAvailable(PastingType::Double); }),
        makeAction("paste-special", [this]() { return m_editMenuController->isPasteAvailable(PastingType::Special); }),
        makeAction("swap", [this]() { return m_editMenuController->isSwapAvailable(); }),
        makeAction("delete", [this]() { return m_editMenuController->isDeleteAvailable(); }),
        makeSeparator(),
        makeAction("select-all", [this]() { return m_editMenuController->isSelectAllAvailable(); }),
        makeAction("select-similar", [this]() { return m_editMenuController->isSelectSimilarAvailable(); }),
        makeAction("find", [this]() { return m_editMenuController->isFindAvailable(); }),
        makeSeparator(),
        makeAction("preference-dialog", [this]() { return m_editMenuController->isPreferenceDialogAvailable(); }) // need implement
    };

    return makeMenu(trc("appshell", "&Edit"), editItems);
}

MenuItem AppMenuModel::viewItem()
{
    MenuItemList viewItems {
        makeAction("toggle-palette", isPanelVisible(PanelType::Palette),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::Palette); }),
        makeAction("toggle-instruments", isPanelVisible(PanelType::Instruments),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::Instruments); }),
        makeAction("masterpalette", paletteController()->isMasterPaletteOpened().val,
                   [this]() { return m_viewMenuController->isMasterPaletteAvailable(); }),
        makeAction("inspector", isPanelVisible(PanelType::Inspector),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::Inspector); }),
        makeAction("toggle-navigator", isPanelVisible(PanelType::NotationNavigator),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::NotationNavigator); }),
        makeAction("toggle-timeline", isPanelVisible(PanelType::TimeLine),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::TimeLine); }), // need implement
        makeAction("toggle-mixer", isPanelVisible(PanelType::Mixer),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::Mixer); }), // need implement
        makeAction("synth-control", isPanelVisible(PanelType::Synthesizer),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::Synthesizer); }), // need implement
        makeAction("toggle-selection-window", isPanelVisible(PanelType::SelectionFilter),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::SelectionFilter); }), // need implement
        makeAction("toggle-piano", isPanelVisible(PanelType::Piano),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::Piano); }), // need implement
        makeAction("toggle-scorecmp-tool", isPanelVisible(PanelType::ComparisonTool),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::ComparisonTool); }), // need implement
        makeSeparator(),
        makeAction("zoomin", [this]() { return m_viewMenuController->isZoomInAvailable(); }),
        makeAction("zoomout", [this]() { return m_viewMenuController->isZoomInAvailable(); }),
        makeSeparator(),
        makeMenu(trc("appshell", "&Toolbars"), toolbarsItems()),
        makeMenu(trc("appshell", "W&orkspaces"), workspacesItems(), true, "select-workspace"),
        makeAction("toggle-statusbar", isPanelVisible(PanelType::NotationStatusBar),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::NotationStatusBar); }),
        makeSeparator(),
        makeAction("split-h", false, [this]() { return m_viewMenuController->isSplitAvailable(framework::Orientation::Horizontal); }), // need implement
        makeAction("split-v", false, [this]() { return m_viewMenuController->isSplitAvailable(framework::Orientation::Vertical); }), // need implement
        makeSeparator(),
        makeAction("show-invisible", scoreConfig().isShowInvisibleElements,
                   [this]() { return m_viewMenuController->isScoreConfigAvailable(ScoreConfigType::ShowInvisibleElements); }),
        makeAction("show-unprintable", scoreConfig().isShowUnprintableElements,
                   [this]() { return m_viewMenuController->isScoreConfigAvailable(ScoreConfigType::ShowUnprintableElements); }),
        makeAction("show-frames", scoreConfig().isShowFrames,
                   [this]() { return m_viewMenuController->isScoreConfigAvailable(ScoreConfigType::ShowFrames); }),
        makeAction("show-pageborders", scoreConfig().isShowPageMargins,
                   [this]() { return m_viewMenuController->isScoreConfigAvailable(ScoreConfigType::ShowPageMargins); }),
        makeAction("mark-irregular", scoreConfig().isMarkIrregularMeasures,
                   [this]() { return m_viewMenuController->isScoreConfigAvailable(ScoreConfigType::MarkIrregularMeasures); }),
        makeSeparator(),
        makeAction("fullscreen", controller()->isFullScreen().val, [this]() { return m_viewMenuController->isFullScreenAvailable(); })
    };

    return makeMenu(trc("appshell", "&View"), viewItems);
}

MenuItem AppMenuModel::addItem()
{
    MenuItemList addItems {
        makeMenu(trc("appshell", "N&otes"), notesItems()),
        makeMenu(trc("appshell", "&Intervals"), intervalsItems()),
        makeMenu(trc("appshell", "T&uplets"), tupletsItems()),
        makeSeparator(),
        makeMenu(trc("appshell", "&Measures"), measuresItems()),
        makeMenu(trc("appshell", "&Frames"), framesItems()),
        makeMenu(trc("appshell", "&Text"), textItems()),
        makeMenu(trc("appshell", "&Lines"), linesItems()),
    };

    return makeMenu(trc("appshell", "&Add"), addItems, scoreOpened());
}

MenuItem AppMenuModel::formatItem()
{
    MenuItemList stretchItems {
        makeAction("stretch+", selectedRangeOnScore()),
        makeAction("stretch-", selectedRangeOnScore()),
        makeAction("reset-stretch", selectedRangeOnScore())
    };

    MenuItemList formatItems {
        makeAction("edit-style", scoreOpened()),
        makeAction("page-settings", scoreOpened()), // need implement
        makeSeparator(),
        makeMenu(trc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeAction("reset-text-style-overrides", scoreOpened()),
        makeAction("reset-beammode", !hasSelectionOnScore() || selectedRangeOnScore()),
        makeAction("reset", hasSelectionOnScore()),
        makeSeparator(),
        makeAction("load-style", scoreOpened()), // need implement
        makeAction("save-style", scoreOpened()) // need implement
    };

    return makeMenu(trc("appshell", "F&ormat"), formatItems, scoreOpened());
}

MenuItem AppMenuModel::toolsItem()
{
    MenuItemList voicesItems {
        makeAction("voice-x12"),
        makeAction("voice-x13"),
        makeAction("voice-x14"),
        makeAction("voice-x23"),
        makeAction("voice-x24"),
        makeAction("voice-x34")
    };

    MenuItemList measuresItems {
        makeAction("split-measure"),
        makeAction("join-measures"),
    };

    MenuItemList toolsItems {
        makeAction("transpose"),
        makeSeparator(),
        makeAction("explode"),
        makeAction("implode"),
        makeAction("realize-chord-symbols"),
        makeMenu(trc("appshell", "&Voices"), voicesItems),
        makeMenu(trc("appshell", "&Measures"), measuresItems),
        makeAction("time-delete"),
        makeSeparator(),
        makeAction("slash-fill"),
        makeAction("slash-rhythm"),
        makeSeparator(),
        makeAction("pitch-spell"),
        makeAction("reset-groupings"),
        makeAction("resequence-rehearsal-marks"),
        makeAction("unroll-repeats"),
        makeSeparator(),
        makeAction("copy-lyrics-to-clipboard"),
        makeAction("fotomode"), // need implement
        makeAction("del-empty-measures"),
    };

    return makeMenu(trc("appshell", "&Tools"), toolsItems, scoreOpened());
}

MenuItem AppMenuModel::helpItem()
{
    MenuItemList toursItems {
        makeAction("show-tours"), // need implement
        makeAction("reset-tours") // need implement
    };

    MenuItemList helpItems {
        makeAction("online-handbook"),
        makeMenu(trc("appshell", "&Tours"), toursItems),
        makeSeparator(),
        makeAction("about"), // need implement
        makeAction("about-qt"),
        makeAction("about-musicxml"), // need implement
    };

    if (configuration()->isAppUpdatable()) {
        helpItems << makeAction("check-update"); // need implement
    }

    helpItems << makeSeparator()
              << makeAction("ask-help")
              << makeAction("report-bug")
              << makeAction("leave-feedback")
              << makeSeparator()
              << makeAction("revert-factory"); // need implement

    return makeMenu(trc("appshell", "&Help"), helpItems);
}

MenuItemList AppMenuModel::recentScores()
{
    MenuItemList items;
    MetaList recentScores = userScoresService()->recentScoreList().val;
    if (recentScores.empty()) {
        return items;
    }

    for (const Meta& meta: recentScores) {
        MenuItem item = actionsRegister()->action("file-open");
        item.title = !meta.title.isEmpty() ? meta.title.toStdString() : meta.fileName.toStdString();
        item.args = ActionData::make_arg1<io::path>(meta.filePath);
        item.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeAction("clear-recent");

    return items;
}

MenuItemList AppMenuModel::notesItems()
{
    MenuItemList items {
        makeAction("note-input", isNoteInputMode(), [this]() { return m_addMenuController->isNoteInputAvailable(); }),
        makeSeparator(),
        makeAction("note-c", [this]() { return m_addMenuController->isNoteAvailable(NoteName::C, NoteAddingMode::NextChord); }),
        makeAction("note-d", [this]() { return m_addMenuController->isNoteAvailable(NoteName::D, NoteAddingMode::NextChord); }),
        makeAction("note-e", [this]() { return m_addMenuController->isNoteAvailable(NoteName::E, NoteAddingMode::NextChord); }),
        makeAction("note-f", [this]() { return m_addMenuController->isNoteAvailable(NoteName::F, NoteAddingMode::NextChord); }),
        makeAction("note-g", [this]() { return m_addMenuController->isNoteAvailable(NoteName::G, NoteAddingMode::NextChord); }),
        makeAction("note-a", [this]() { return m_addMenuController->isNoteAvailable(NoteName::A, NoteAddingMode::NextChord); }),
        makeAction("note-b", [this]() { return m_addMenuController->isNoteAvailable(NoteName::B, NoteAddingMode::NextChord); }),
        makeSeparator(),
        makeAction("chord-c", [this]() { return m_addMenuController->isNoteAvailable(NoteName::C, NoteAddingMode::CurrentChord); }),
        makeAction("chord-d", [this]() { return m_addMenuController->isNoteAvailable(NoteName::D, NoteAddingMode::CurrentChord); }),
        makeAction("chord-e", [this]() { return m_addMenuController->isNoteAvailable(NoteName::E, NoteAddingMode::CurrentChord); }),
        makeAction("chord-f", [this]() { return m_addMenuController->isNoteAvailable(NoteName::F, NoteAddingMode::CurrentChord); }),
        makeAction("chord-g", [this]() { return m_addMenuController->isNoteAvailable(NoteName::G, NoteAddingMode::CurrentChord); }),
        makeAction("chord-a", [this]() { return m_addMenuController->isNoteAvailable(NoteName::A, NoteAddingMode::CurrentChord); }),
        makeAction("chord-b", [this]() { return m_addMenuController->isNoteAvailable(NoteName::G, NoteAddingMode::CurrentChord); })
    };

    return items;
}

MenuItemList AppMenuModel::intervalsItems()
{
    MenuItemList items {
        makeAction("interval1", [this]() { return m_addMenuController->isIntervalAvailable(1, IntervalType::Above); }),
        makeAction("interval2", [this]() { return m_addMenuController->isIntervalAvailable(2, IntervalType::Above); }),
        makeAction("interval3", [this]() { return m_addMenuController->isIntervalAvailable(3, IntervalType::Above); }),
        makeAction("interval4", [this]() { return m_addMenuController->isIntervalAvailable(4, IntervalType::Above); }),
        makeAction("interval5", [this]() { return m_addMenuController->isIntervalAvailable(5, IntervalType::Above); }),
        makeAction("interval6", [this]() { return m_addMenuController->isIntervalAvailable(6, IntervalType::Above); }),
        makeAction("interval7", [this]() { return m_addMenuController->isIntervalAvailable(7, IntervalType::Above); }),
        makeAction("interval8", [this]() { return m_addMenuController->isIntervalAvailable(8, IntervalType::Above); }),
        makeAction("interval9", [this]() { return m_addMenuController->isIntervalAvailable(9, IntervalType::Above); }),
        makeSeparator(),
        makeAction("interval-2", [this]() { return m_addMenuController->isIntervalAvailable(2, IntervalType::Below); }),
        makeAction("interval-3", [this]() { return m_addMenuController->isIntervalAvailable(3, IntervalType::Below); }),
        makeAction("interval-4", [this]() { return m_addMenuController->isIntervalAvailable(4, IntervalType::Below); }),
        makeAction("interval-5", [this]() { return m_addMenuController->isIntervalAvailable(5, IntervalType::Below); }),
        makeAction("interval-6", [this]() { return m_addMenuController->isIntervalAvailable(6, IntervalType::Below); }),
        makeAction("interval-7", [this]() { return m_addMenuController->isIntervalAvailable(7, IntervalType::Below); }),
        makeAction("interval-8", [this]() { return m_addMenuController->isIntervalAvailable(8, IntervalType::Below); }),
        makeAction("interval-9", [this]() { return m_addMenuController->isIntervalAvailable(9, IntervalType::Below); })
    };

    return items;
}

MenuItemList AppMenuModel::tupletsItems()
{
    MenuItemList items {
        makeAction("duplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Duplet); }),
        makeAction("triplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Triplet); }),
        makeAction("quadruplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Quadruplet); }),
        makeAction("quintuplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Quintuplet); }),
        makeAction("sextuplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Sextuplet); }),
        makeAction("septuplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Septuplet); }),
        makeAction("octuplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Octuplet); }),
        makeAction("nonuplet", [this]() { return m_addMenuController->isTupletAvailable(TupletType::Nonuplet); }),
        makeAction("tuplet-dialog", [this]() { return m_addMenuController->isTupletDialogAvailable(); })
    };

    return items;
}

MenuItemList AppMenuModel::measuresItems()
{
    MenuItemList items {
        makeAction("insert-measure", [this]() { return m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Insert, 0); }),
        makeAction("insert-measures", [this]() { return m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Insert, 2); }),
        makeSeparator(),
        makeAction("append-measure", [this]() { return m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Append, 0); }),
        makeAction("append-measures", [this]() { return m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Append, 2); })
    };

    return items;
}

MenuItemList AppMenuModel::framesItems()
{
    MenuItemList items {
        makeAction("insert-hbox", [this]() {
            return m_addMenuController->isBoxAvailable(ElementChangeOperation::Insert, BoxType::Horizontal);
        }),
        makeAction("insert-vbox", [this]() {
            return m_addMenuController->isBoxAvailable(ElementChangeOperation::Insert, BoxType::Vertical);
        }),
        makeAction("insert-textframe", [this]() {
            return m_addMenuController->isBoxAvailable(ElementChangeOperation::Insert, BoxType::Text);
        }),
        makeSeparator(),
        makeAction("append-hbox", [this]() {
            return m_addMenuController->isBoxAvailable(ElementChangeOperation::Append, BoxType::Horizontal);
        }),
        makeAction("append-vbox", [this]() {
            return m_addMenuController->isBoxAvailable(ElementChangeOperation::Append, BoxType::Vertical);
        }),
        makeAction("append-textframe", [this]() {
            return m_addMenuController->isBoxAvailable(ElementChangeOperation::Append, BoxType::Text);
        })
    };

    return items;
}

MenuItemList AppMenuModel::textItems()
{
    MenuItemList items {
        makeAction("title-text", [this]() { return m_addMenuController->isTextAvailable(TextType::TITLE); }),
        makeAction("subtitle-text", [this]() { return m_addMenuController->isTextAvailable(TextType::SUBTITLE); }),
        makeAction("composer-text", [this]() { return m_addMenuController->isTextAvailable(TextType::COMPOSER); }),
        makeAction("poet-text", [this]() { return m_addMenuController->isTextAvailable(TextType::POET); }),
        makeAction("part-text", [this]() { return m_addMenuController->isTextAvailable(TextType::INSTRUMENT_EXCERPT); }),
        makeSeparator(),
        makeAction("system-text", [this]() { return m_addMenuController->isTextAvailable(TextType::SYSTEM); }),
        makeAction("staff-text", [this]() { return m_addMenuController->isTextAvailable(TextType::STAFF); }),
        makeAction("expression-text", [this]() { return m_addMenuController->isTextAvailable(TextType::EXPRESSION); }),
        makeAction("rehearsalmark-text", [this]() { return m_addMenuController->isTextAvailable(TextType::REHEARSAL_MARK); }),
        makeAction("instrument-change-text", [this]() { return m_addMenuController->isTextAvailable(TextType::INSTRUMENT_CHANGE); }),
        makeAction("fingering-text", [this]() { return m_addMenuController->isTextAvailable(TextType::FINGERING); }),
        makeSeparator(),
        makeAction("sticking-text", [this]() { return m_addMenuController->isTextAvailable(TextType::STICKING); }),
        makeAction("chord-text", [this]() { return m_addMenuController->isTextAvailable(TextType::HARMONY_A); }),
        makeAction("roman-numeral-text", [this]() { return m_addMenuController->isTextAvailable(TextType::HARMONY_ROMAN); }),
        makeAction("nashville-number-text", [this]() { return m_addMenuController->isTextAvailable(TextType::HARMONY_NASHVILLE); }),
        makeAction("lyrics", [this]() { return m_addMenuController->isTextAvailable(TextType::LYRICS_ODD); }),
        makeAction("figured-bass", [this]() { return m_addMenuController->isFiguredBassAvailable(); }),
        makeAction("tempo", [this]() { return m_addMenuController->isTextAvailable(TextType::TEMPO); })
    };

    return items;
}

MenuItemList AppMenuModel::linesItems()
{
    MenuItemList items {
        makeAction("add-slur", [this]() { return m_addMenuController->isSlurAvailable(); }),
        makeAction("add-hairpin", [this]() { return m_addMenuController->isHarpinAvailable(HairpinType::CRESC_HAIRPIN); }),
        makeAction("add-hairpin-reverse", [this]() { return m_addMenuController->isHarpinAvailable(HairpinType::DECRESC_HAIRPIN); }),
        makeAction("add-8va", [this]() { return m_addMenuController->isOttavaAvailable(OttavaType::OTTAVA_8VA); }),
        makeAction("add-8vb", [this]() { return m_addMenuController->isOttavaAvailable(OttavaType::OTTAVA_8VB); }),
        makeAction("add-noteline", [this]() { return m_addMenuController->isNoteLineAvailable(); })
    };

    return items;
}

MenuItemList AppMenuModel::toolbarsItems()
{
    MenuItemList items {
        makeAction("toggle-transport", isPanelVisible(PanelType::PlaybackToolBar),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::PlaybackToolBar); }),
        makeAction("toggle-noteinput", isPanelVisible(PanelType::NoteInputBar),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::NoteInputBar); }),
        makeAction("toggle-notationtoolbar", isPanelVisible(PanelType::NotationToolBar),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::NotationToolBar); }),
        makeAction("toggle-undoredo", isPanelVisible(PanelType::UndoRedoToolBar),
                   [this]() { return m_viewMenuController->isPanelAvailable(PanelType::UndoRedoToolBar); })
    };

    return items;
}

MenuItemList AppMenuModel::workspacesItems()
{
    MenuItemList items;

    RetVal<IWorkspacePtrList> workspaces = workspacesManager()->workspaces();
    if (!workspaces.ret) {
        return items;
    }

    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace().val;

    std::sort(workspaces.val.begin(), workspaces.val.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    for (const IWorkspacePtr& workspace : workspaces.val) {
        MenuItem item = actionsRegister()->action("select-workspace");
        item.title = workspace->title();
        item.args = ActionData::make_arg1<std::string>(workspace->name());

        bool isCurrentWorkspace = workspace == currentWorkspace;
        item.checked = isCurrentWorkspace;
        item.checkable = true;
        item.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeAction("configure-workspaces");

    return items;
}

bool AppMenuModel::isPanelVisible(PanelType type) const
{
    return notationPageState()->isPanelVisible(type);
}

bool AppMenuModel::needSaveScore() const
{
    return currentMasterNotation() && currentMasterNotation()->needSave().val;
}

bool AppMenuModel::scoreOpened() const
{
    return currentNotation() != nullptr;
}

bool AppMenuModel::canUndo() const
{
    return currentNotation() ? currentNotation()->undoStack()->canUndo() : false;
}

bool AppMenuModel::canRedo() const
{
    return currentNotation() ? currentNotation()->undoStack()->canRedo() : false;
}

bool AppMenuModel::selectedElementOnScore() const
{
    return notationSelection() ? notationSelection()->element() != nullptr : false;
}

bool AppMenuModel::selectedRangeOnScore() const
{
    return notationSelection() ? notationSelection()->isRange() : false;
}

bool AppMenuModel::hasSelectionOnScore() const
{
    return notationSelection() ? !notationSelection()->isNone() : false;
}

bool AppMenuModel::isNoteInputMode() const
{
    return currentNotation() ? currentNotation()->interaction()->noteInput()->isNoteInputMode() : false;
}

bool AppMenuModel::isNotationPage() const
{
    return interactive()->isOpened("musescore://notation").val;
}

ScoreConfig AppMenuModel::scoreConfig() const
{
    return currentNotation() ? currentNotation()->interaction()->scoreConfig() : ScoreConfig();
}
