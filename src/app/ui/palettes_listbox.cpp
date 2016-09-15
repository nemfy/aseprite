// Aseprite
// Copyright (C) 2001-2016  David Capello
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/palettes_listbox.h"

#include "app/modules/palettes.h"
#include "app/res/palette_resource.h"
#include "app/res/palettes_loader_delegate.h"
#include "app/ui/icon_button.h"
#include "app/ui/skin/skin_theme.h"
#include "base/bind.h"
#include "base/launcher.h"
#include "doc/palette.h"
#include "she/surface.h"
#include "ui/graphics.h"
#include "ui/listitem.h"
#include "ui/message.h"
#include "ui/paint_event.h"
#include "ui/resize_event.h"
#include "ui/size_hint_event.h"
#include "ui/tooltips.h"
#include "ui/view.h"

namespace app {

using namespace ui;
using namespace app::skin;

static bool is_url_char(int chr)
{
  return ((chr >= 'a' && chr <= 'z') ||
          (chr >= 'A' && chr <= 'Z') ||
          (chr >= '0' && chr <= '9') ||
          (chr == ':' || chr == '/' || chr == '@' ||
           chr == '?' || chr == '!' || chr == '#' ||
           chr == '-' || chr == '_' || chr == '~' ||
           chr == '.' || chr == ',' || chr == ';' ||
           chr == '*' || chr == '+' || chr == '=' ||
           chr == '[' || chr == ']' ||
           chr == '(' || chr == ')' ||
           chr == '$' || chr == '\''));
}

class PalettesListItem : public ResourceListItem {

  class CommentButton : public IconButton {
  public:
    CommentButton(const std::string& comment)
      : IconButton(SkinTheme::instance()->parts.iconUserData()->bitmap(0))
      , m_comment(comment) {
    }

  private:
    void onClick(Event& ev) override {
      IconButton::onClick(ev);

      int j, i = m_comment.find("http");
      if (i != std::string::npos) {
        for (j=i+4; j<m_comment.size() && is_url_char(m_comment[j]); ++j)
          ;
        base::launcher::open_url(m_comment.substr(i, j-i));
      }
    }

    std::string m_comment;
  };

public:
  PalettesListItem(Resource* resource, TooltipManager* tooltips)
    : ResourceListItem(resource)
    , m_comment(nullptr)
  {
    std::string comment = static_cast<PaletteResource*>(resource)->palette()->comment();
    if (!comment.empty()) {
      addChild(m_comment = new CommentButton(comment));

      m_comment->setBgColor(SkinTheme::instance()->colors.listitemNormalFace());
      tooltips->addTooltipFor(m_comment, comment, LEFT);
    }
  }

private:
  void onResize(ResizeEvent& ev) override {
    ResourceListItem::onResize(ev);

    if (m_comment) {
      auto reqSz = m_comment->sizeHint();
      m_comment->setBounds(
        gfx::Rect(ev.bounds().x+ev.bounds().w-reqSz.w,
                  ev.bounds().y+ev.bounds().h/2-reqSz.h/2,
                  reqSz.w, reqSz.h));
    }
  }

  CommentButton* m_comment;
};

PalettesListBox::PalettesListBox()
  : ResourcesListBox(new ResourcesLoader(new PalettesLoaderDelegate))
{
  addChild(&m_tooltips);
}

doc::Palette* PalettesListBox::selectedPalette()
{
  Resource* resource = selectedResource();
  if (!resource)
    return NULL;

  return static_cast<PaletteResource*>(resource)->palette();
}

ResourceListItem* PalettesListBox::onCreateResourceItem(Resource* resource)
{
  return new PalettesListItem(resource, &m_tooltips);
}

void PalettesListBox::onResourceChange(Resource* resource)
{
  doc::Palette* palette = static_cast<PaletteResource*>(resource)->palette();
  PalChange(palette);
}

void PalettesListBox::onPaintResource(Graphics* g, const gfx::Rect& bounds, Resource* resource)
{
  doc::Palette* palette = static_cast<PaletteResource*>(resource)->palette();

  gfx::Rect box(
    bounds.x, bounds.y+bounds.h-6*guiscale(),
    4*guiscale(), 4*guiscale());

  for (int i=0; i<palette->size(); ++i) {
    doc::color_t c = palette->getEntry(i);

    g->fillRect(gfx::rgba(
        doc::rgba_getr(c),
        doc::rgba_getg(c),
        doc::rgba_getb(c)), box);

    box.x += box.w;
  }
}

void PalettesListBox::onResourceSizeHint(Resource* resource, gfx::Size& size)
{
  size = gfx::Size(0, (2+16+2)*guiscale());
}

} // namespace app
