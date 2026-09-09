#include "RoundedRaffTheme.h"

#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HalGPIO.h>
#include <HalStorage.h>
#include <I18n.h>

#include <algorithm>
#include <string>
#include <vector>
#include "RecentBooksStore.h"
#include "components/UITheme.h"
#include "components/icons/book.h"
#include "components/icons/cover.h"
#include "components/icons/folder.h"
#include "components/icons/library.h"
#include "components/icons/recent.h"
#include "components/icons/settings2.h"
#include "components/icons/transfer.h"
#include "fontIds.h"

namespace {

constexpr int kCalendarRadius = 16;
constexpr int kCardRadius = 18;
constexpr int kMenuRadius = 12;
constexpr int kHeadingFont = UI_12_FONT_ID;
constexpr int kBodyFont = UI_10_FONT_ID;
constexpr int kSmallFont = SMALL_FONT_ID;
constexpr int kGuideFontId = SMALL_FONT_ID;

const uint8_t* iconBitmap(const UIIcon icon) {
  switch (icon) {
    case Folder: return FolderIcon;
    case Book: return BookIcon;
    case Recent: return RecentIcon;
    case Settings: return Settings2Icon;
    case Transfer: return TransferIcon;
    case Library: return LibraryIcon;
    default: return nullptr;
  }
}
void drawCentered(GfxRenderer& r, int fontId, int y, const char* text, bool bold = false) {
  if (!text || !*text) return;
  const auto style = bold ? EpdFontFamily::BOLD : EpdFontFamily::REGULAR;
  const int width = r.getTextWidth(fontId, text, style);
  r.drawText(fontId, (r.getScreenWidth() - width) / 2, y, text, true, style);
}

void drawCalendar(GfxRenderer& r, const HomeRenderContext& h) {
  const int x = 20;
  const int y = 13;
  const int w = r.getScreenWidth() - 40;
  const int height = 167;
  r.fillRoundedRect(x, y, w, height, kCalendarRadius, Color::White);
  r.drawRoundedRect(x, y, w, height, 2, kCalendarRadius, true);

  // Deliberately fixed to the agreed X4 visual prototype.
  drawCentered(r, kHeadingFont, 26, h.calendarMonth ? h.calendarMonth : "THÁNG 9", true);
  if (h.calendarDay && *h.calendarDay) {
    const int dayWidth = r.getTextWidth(NOTOSANS_18_FONT_ID, h.calendarDay, EpdFontFamily::BOLD);
    r.drawText(NOTOSANS_18_FONT_ID, (r.getScreenWidth() - dayWidth) / 2, 55, h.calendarDay, true,
               EpdFontFamily::BOLD);
  }

  drawCentered(r, kHeadingFont, 114, h.calendarWeekday ? h.calendarWeekday : "THỨ SÁU", true);
  drawCentered(r, kBodyFont, 145, h.calendarLunar ? h.calendarLunar : "Âm lịch: 23 tháng 7");
}
void drawOwner(GfxRenderer& r, int x, int y, int width) {
  constexpr const char* kName = "Bs Phương";
  constexpr const char* kPhone = "0843191075";
  constexpr const char* kEmail = "nhakhoaphuong@gmail.com";

  r.drawText(kBodyFont, x, y, kName, true);
  r.drawText(kBodyFont, x, y + 23, kPhone, true);
  const std::string email = r.truncatedText(kBodyFont, kEmail, width);
  r.drawText(kBodyFont, x, y + 46, email.c_str(), true);
}
void drawDemoCover(GfxRenderer& r, int x, int y, int w, int h) {
  r.fillRoundedRect(x, y, w, h, 14, Color::LightGray);
  r.drawRoundedRect(x, y, w, h, 2, 14, true);

  drawCentered(r, kBodyFont, y + 30, "DOUGLAS", true);
  drawCentered(r, kBodyFont, y + 53, "ADAMS", true);

  r.drawLine(x + 16, y + 74, x + w - 16, y + 74, 1, true);
  drawCentered(r, kHeadingFont, y + 102, "THE", true);
  drawCentered(r, kHeadingFont, y + 130, "HITCHHIKER'S", true);
  drawCentered(r, kHeadingFont, y + 158, "GUIDE", true);
  r.drawLine(x + 24, y + h - 58, x + w - 24, y + h - 58, 1, true);
  drawCentered(r, kSmallFont, y + h - 42, "TO THE GALAXY");
}

void drawBookCard(GfxRenderer& r, const HomeRenderContext& h) {
  const int x = 20;
  const int y = 190;
  const int w = r.getScreenWidth() - 40;
  const int height = 470;

  r.fillRoundedRect(x, y, w, height, kCardRadius, Color::White);
  r.drawRoundedRect(x, y, w, height, 2, kCardRadius, true);
  // Two equal visual columns.
  const int dividerX = 240;
  r.drawLine(dividerX, y + 16, dividerX, y + height - 16, 1, true);

  // Column headings.
  r.drawText(kHeadingFont, 34, y + 18, "ĐANG ĐỌC", true, EpdFontFamily::BOLD);
  r.drawText(kHeadingFont, 260, y + 18, "THÔNG TIN", true, EpdFontFamily::BOLD);

  const int coverX = 40;
  const int coverY = y + 55;
  const int coverW = 175;
  const int coverH = 280;
  if (!h.recentBooks.empty()) {
    const RecentBook& book = h.recentBooks.front();
    bool hasCover = false;
    if (!book.coverBmpPath.empty()) {
      const std::string path = UITheme::getCoverThumbPath(book.coverBmpPath, coverH);
      HalFile file;
      if (Storage.openFileForRead("HOME", path, file)) {
        Bitmap bitmap(file);
        if (bitmap.parseHeaders() == BmpReaderError::Ok) {
          const int actualW = std::min(bitmap.getWidth(), coverW);
          const int cx = coverX + (coverW - actualW) / 2;
          r.drawBitmap(bitmap, cx, coverY, actualW, coverH);
          r.maskRoundedRectOutsideCorners(cx, coverY, actualW, coverH, 14, Color::White);
          hasCover = true;
        }
        file.close();
      }
    }
    if (!hasCover) drawDemoCover(r, coverX, coverY, coverW, coverH);

    const int infoX = 260;
    const int infoW = 190;
    const auto titleLines = r.wrappedText(kHeadingFont, book.title.c_str(), infoW, 2, EpdFontFamily::BOLD);
    int textY = y + 58;
    for (const auto& line : titleLines) {
      r.drawText(kHeadingFont, infoX, textY, line.c_str(), true, EpdFontFamily::BOLD);
      textY += r.getLineHeight(kHeadingFont);
    }
    if (!book.author.empty()) {
      textY += 3;
      const std::string author = r.truncatedText(kBodyFont, book.author.c_str(), infoW);
      r.drawText(kBodyFont, infoX, textY, author.c_str(), true);
    }

    textY = y + 170;
    r.drawText(kBodyFont, infoX, textY, "Tiến trình đọc", true, EpdFontFamily::BOLD);
    if (h.readingProgressValid) {
      const int pct = std::clamp(h.readingPercentage, 0, 100);
      const int barY = textY + 25;
      const int barW = infoW;
      const int barH = 10;
      r.drawRoundedRect(infoX, barY, barW, barH, 1, 5, true);
      const int fillW = std::max(1, (barW - 2) * pct / 100);
      r.fillRoundedRect(infoX + 1, barY + 1, fillW, barH - 2, 1, Color::Black);
      const std::string pctText = std::to_string(pct) + "%";
      r.drawText(kBodyFont, infoX, barY + 23, pctText.c_str(), true, EpdFontFamily::BOLD);

      const std::string pages =
          std::to_string(h.readingCurrentPage) + " / " + std::to_string(h.readingTotalPages) + " trang";
      const int pageWidth = r.getTextWidth(kBodyFont, pages.c_str());
      r.drawText(kBodyFont, infoX + infoW - pageWidth, barY + 23, pages.c_str(), true);
    }
    r.drawLine(infoX, y + 350, infoX + infoW, y + 350, 1, true);
    drawOwner(r, infoX, y + 374, infoW);
    return;
  }

  // Visual prototype fallback: always show a complete card even before a book
  // is available. Real data wiring will replace this fallback later.
  drawDemoCover(r, coverX, coverY, coverW, coverH);
  const int infoX = 260;
  const int infoW = 190;
  r.drawText(kHeadingFont, infoX, y + 58, "The Hitchhiker's", true, EpdFontFamily::BOLD);
  r.drawText(kHeadingFont, infoX, y + 82, "Guide to the Galaxy", true, EpdFontFamily::BOLD);
  r.drawText(kBodyFont, infoX, y + 112, "Douglas Adams", true);

  r.drawText(kBodyFont, infoX, y + 170, "Tiến trình đọc", true, EpdFontFamily::BOLD);
  const int barY = y + 195;
  const int barW = infoW;
  const int barH = 10;
  const int pct = 62;
  r.drawRoundedRect(infoX, barY, barW, barH, 1, 5, true);
  r.fillRoundedRect(infoX + 1, barY + 1, (barW - 2) * pct / 100, barH - 2, 1, Color::Black);
  r.drawText(kBodyFont, infoX, barY + 23, "62%", true, EpdFontFamily::BOLD);
  r.drawText(kBodyFont, infoX + 94, barY + 23, "173 / 278 trang", true);

  r.drawLine(infoX, y + 350, infoX + infoW, y + 350, 1, true);
  drawOwner(r, infoX, y + 374, infoW);
}
void drawHomeMenu(GfxRenderer& r, const HomeRenderContext& h) {
  const int side = 20;
  const int gap = 6;
  const int top = 680;
  const int height = 72;
  const int count = std::max(1, h.menuCount);
  const int available = r.getScreenWidth() - side * 2;
  const int buttonW = std::max(1, (available - gap * (count - 1)) / count);

  for (int i = 0; i < h.menuCount; ++i) {
    const int bx = side + i * (buttonW + gap);
    const bool selected = h.selectorIndex == i;
    r.fillRoundedRect(bx, top, buttonW, height, kMenuRadius, selected ? Color::Black : Color::White);
    r.drawRoundedRect(bx, top, buttonW, height, 1, kMenuRadius, true);

    const uint8_t* icon = iconBitmap(h.rowIcon(i));
    if (icon) {
      constexpr int iconSize = 20;
      r.drawIcon(icon, bx + (buttonW - iconSize) / 2, top + 9, iconSize);
    }
    const std::string label = h.menuLabel(i);
    const int textWidth = buttonW - 6;
    const auto lines = r.wrappedText(kBodyFont, label.c_str(), textWidth, 2, EpdFontFamily::BOLD);
    const int lineHeight = r.getLineHeight(kBodyFont);
    const int textTop = top + 37 - (static_cast<int>(lines.size()) - 1) * lineHeight / 2;
    for (int line = 0; line < static_cast<int>(lines.size()); ++line) {
      const int tw = r.getTextWidth(kBodyFont, lines[line].c_str(), EpdFontFamily::BOLD);
      r.drawText(kBodyFont, bx + (buttonW - tw) / 2, textTop + line * lineHeight, lines[line].c_str(),
                 !selected, EpdFontFamily::BOLD);
    }
  }
}

}  // namespace
void RoundedRaffTheme::drawHeader(const GfxRenderer& renderer, Rect rect, const char* title,
                                  const char* subtitle) const {
  BaseTheme::drawHeader(renderer, rect, title, subtitle);
}
void RoundedRaffTheme::drawHome(GfxRenderer& renderer, const HomeRenderContext& home) const {
  // Visual-first Home prototype.
  // The standard HomeActivity still owns data/navigation; this method owns
  // the complete RoundedRaff composition and its geometry.
  drawCalendar(renderer, home);
  drawBookCard(renderer, home);
  drawHomeMenu(renderer, home);
  // The HomeActivity's old cover-buffer path is intentionally not used here.
  // The prototype redraws the complete card deterministically on every render.
}
void RoundedRaffTheme::drawRecentBookCover(GfxRenderer& renderer, Rect rect,
                                           const std::vector<RecentBook>& recentBooks, const int selectorIndex,
                                           bool& coverRendered, bool& coverBufferStored, bool& bufferRestored,
                                           std::function<bool()> storeCoverBuffer) const {
  BaseTheme::drawRecentBookCover(renderer, rect, recentBooks, selectorIndex, coverRendered,
                                 coverBufferStored, bufferRestored, std::move(storeCoverBuffer));
}
int RoundedRaffTheme::getMenuRowHeight(const GfxRenderer& renderer) const {
  return renderer.getLineHeight(kHeadingFont) + 20;
}

void RoundedRaffTheme::drawButtonMenu(
    GfxRenderer& renderer, Rect rect, int buttonCount, int selectedIndex,
    const std::function<std::string(int index)>& buttonLabel,
    const std::function<UIIcon(int index)>& rowIcon) const {
  BaseTheme::drawButtonMenu(renderer, rect, buttonCount, selectedIndex, buttonLabel, rowIcon);
}
void RoundedRaffTheme::drawTextField(const GfxRenderer& renderer, Rect rect, const int textWidth, bool cursorMode,
                                     int contentStartX, int contentWidth) const {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int lineHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int lineY = rect.y + rect.height + lineHeight + metrics.verticalSpacing;
  const int thickness = cursorMode ? 3 : 2;
  if (contentWidth > 0) {
    renderer.drawLine(rect.x + contentStartX, lineY, rect.x + contentStartX + contentWidth - 1, lineY, thickness,
                      true);
    return;
  }

  constexpr int hPadding = 8;
  const int lineW = textWidth + hPadding * 2;
  const int lineStart = rect.x + (rect.width - lineW) / 2;
  renderer.drawLine(lineStart, lineY, lineStart + lineW - 1, lineY, thickness, true);
}
void RoundedRaffTheme::drawButtonHints(GfxRenderer& renderer, const char* btn1, const char* btn2, const char* btn3,
                                       const char* btn4) const {
  if (gpio.hasTouch()) return;

  const auto origOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();
  const int sidePadding = 20;
  const int groupGap = 10;
  const int bottomMargin = 4;
  const int hintHeight = RoundedRaffMetrics::values.buttonHintsHeight - 8;
  const int groupWidth = (pageWidth - sidePadding * 2 - groupGap) / 2;
  const int hintY = pageHeight - hintHeight - bottomMargin;
  const int textY = hintY + (hintHeight - renderer.getLineHeight(kGuideFontId)) / 2;
  const bool backDisabled = (btn1 == nullptr || btn1[0] == '\0');
  const int leftGroupX = sidePadding;
  const int rightGroupX = leftGroupX + groupWidth + groupGap;

  const std::string backLabel = backDisabled ? "" : std::string(btn1);
  const std::string selectText = (btn2 && btn2[0] != '\0') ? std::string(btn2) : "";
  const std::string upText = (btn3 && btn3[0] != '\0') ? std::string(btn3) : "";
  const std::string downText = (btn4 && btn4[0] != '\0') ? std::string(btn4) : "";
  renderer.drawRoundedRect(leftGroupX, hintY, groupWidth, hintHeight, 1, 14, true);
  renderer.drawRoundedRect(rightGroupX, hintY, groupWidth, hintHeight, 1, 14, true);

  constexpr int inner = 14;
  const int selectWidth = renderer.getTextWidth(kGuideFontId, selectText.c_str(), EpdFontFamily::REGULAR);
  const int downWidth = renderer.getTextWidth(kGuideFontId, downText.c_str(), EpdFontFamily::REGULAR);
  if (!backDisabled) {
    renderer.drawText(kGuideFontId, leftGroupX + inner, textY, backLabel.c_str(), true, EpdFontFamily::REGULAR);
  }
  renderer.drawText(kGuideFontId, leftGroupX + groupWidth - inner - selectWidth, textY, selectText.c_str(), true,
                    EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, rightGroupX + inner, textY, upText.c_str(), true, EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, rightGroupX + groupWidth - inner - downWidth, textY, downText.c_str(), true,
                    EpdFontFamily::REGULAR);
  renderer.setOrientation(origOrientation);
}
