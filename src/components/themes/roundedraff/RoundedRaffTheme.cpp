#include "RoundedRaffTheme.h"

#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HalGPIO.h>
#include <HalStorage.h>
#include <I18n.h>
#include <algorithm>
#include <cstdlib>
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
#include "calendarDigits.h"

namespace {
constexpr int kCalendarRadius = 16;
constexpr int kCardRadius = 18;
constexpr int kMenuRadius = 12;
// CrossPoint font hierarchy: use UI_12 only for section headings.
// Dense Home content uses SMALL_FONT_ID so every string can be measured
// against its actual rendered width before drawing.
constexpr int kHeadingFont = UI_12_FONT_ID;
constexpr int kInfoFont = SMALL_FONT_ID;
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
// Draw text centered inside an explicit logical rectangle.
// Never use screen-wide centering for text belonging to a sub-panel.
void drawCenteredInBox(GfxRenderer& r, int fontId, int x, int width, int y, const char* text,
                       bool bold = false) {
  if (!text || !*text || width <= 0) return;
  const auto style = bold ? EpdFontFamily::BOLD : EpdFontFamily::REGULAR;
  const std::string safe = r.truncatedText(fontId, text, width, style);
  const int textWidth = r.getTextWidth(fontId, safe.c_str(), style);
  const int drawX = x + std::max(0, (width - textWidth) / 2);
  r.drawText(fontId, drawX, y, safe.c_str(), true, style);
}
void drawCalendar(GfxRenderer& r, const HomeRenderContext& h) {
  const int x = 20;
  const int y = 10;
  const int w = r.getScreenWidth() - 40;
  const int height = 300;

  r.fillRoundedRect(x, y, w, height, kCalendarRadius, Color::White);
  r.drawRoundedRect(x, y, w, height, 2, kCalendarRadius, true);
  drawCenteredInBox(r, UI_12_FONT_ID, x + 8, w - 16, y + 34,
                    h.calendarMonth ? h.calendarMonth : "THÁNG 9", true);

  // The solar day is the focal calendar element. Render it exclusively from
  // the supplied 32x48 monochrome bitmap digits and center the complete group.
  int dayValue = 1;
  if (h.calendarDay && *h.calendarDay) dayValue = std::atoi(h.calendarDay);
  dayValue = std::clamp(dayValue, 1, 31);
  const int tens = dayValue / 10;
  const int ones = dayValue % 10;
  constexpr int digitW = 32;
  constexpr int digitH = 48;
  constexpr int digitGap = 4;
  const int digitCount = dayValue >= 10 ? 2 : 1;
  const int groupW = digitCount * digitW + (digitCount - 1) * digitGap;
  const int groupX = x + (w - groupW) / 2;
  const auto digitBitmap = [](int digit) -> const uint8_t* {
    static const uint8_t* const digits[] = {
        CalendarDayDigit0, CalendarDayDigit1, CalendarDayDigit2, CalendarDayDigit3, CalendarDayDigit4,
        CalendarDayDigit5, CalendarDayDigit6, CalendarDayDigit7, CalendarDayDigit8, CalendarDayDigit9};
    return digits[std::clamp(digit, 0, 9)];
  };
  const int dayY = y + 112;
  if (digitCount == 1) {
    r.drawImage(digitBitmap(ones), groupX, dayY, digitW, digitH);
  } else {
    r.drawImage(digitBitmap(tens), groupX, dayY, digitW, digitH);
    r.drawImage(digitBitmap(ones), groupX + digitW + digitGap, dayY, digitW, digitH);
  }

  drawCenteredInBox(r, UI_12_FONT_ID, x + 8, w - 16, y + 194,
                    h.calendarWeekday ? h.calendarWeekday : "THỨ SÁU", true);
  drawCenteredInBox(r, UI_10_FONT_ID, x + 8, w - 16, y + 226,
                    h.calendarLunar ? h.calendarLunar : "Âm lịch: 23 tháng 7");
}

void drawOwner(GfxRenderer& r, int x, int y, int width,
                const HomeRenderContext& h) {
  const char* kName = h.ownerName && *h.ownerName ? h.ownerName : "";
  const char* kPhone = h.ownerPhone && *h.ownerPhone ? h.ownerPhone : "";
  const char* kEmail = h.ownerEmail && *h.ownerEmail ? h.ownerEmail : "";
  // Owner is deliberately quiet: one size smaller than UI_10,
  // regular text, not bold.
  constexpr int ownerFont = SMALL_FONT_ID;
  constexpr int ownerLineGap = 14;

  const std::string name =
      r.truncatedText(ownerFont, kName, width, EpdFontFamily::REGULAR);
  const std::string phone =
      r.truncatedText(ownerFont, kPhone, width, EpdFontFamily::REGULAR);
  const std::string email =
      r.truncatedText(ownerFont, kEmail, width, EpdFontFamily::REGULAR);
  r.drawText(ownerFont, x, y, name.c_str(), true, EpdFontFamily::REGULAR);
  r.drawText(ownerFont, x, y + ownerLineGap, phone.c_str(), true,
             EpdFontFamily::REGULAR);
  r.drawText(ownerFont, x, y + ownerLineGap * 2, email.c_str(), true,
             EpdFontFamily::REGULAR);
}
void drawDemoCover(GfxRenderer& r, int x, int y, int w, int h) {
  r.fillRoundedRect(x, y, w, h, 14, Color::LightGray);
  r.drawRoundedRect(x, y, w, h, 2, 14, true);
  // Every line is measured inside the cover width. Nothing is allowed to
  // escape into the information column.
  drawCenteredInBox(r, kInfoFont, x + 8, w - 16, y + 30, "DOUGLAS", true);
  drawCenteredInBox(r, kInfoFont, x + 8, w - 16, y + 50, "ADAMS", true);
  r.drawLine(x + 16, y + 72, x + w - 16, y + 72, 1, true);
  drawCenteredInBox(r, kInfoFont, x + 8, w - 16, y + 101, "THE", true);
  drawCenteredInBox(r, kInfoFont, x + 8, w - 16, y + 121, "HITCHHIKER'S", true);
  drawCenteredInBox(r, kInfoFont, x + 8, w - 16, y + 141, "GUIDE", true);

  r.drawLine(x + 24, y + h - 58, x + w - 24, y + h - 58, 1, true);
  drawCenteredInBox(r, kSmallFont, x + 8, w - 16, y + h - 42, "TO THE GALAXY");
}
void drawBookInfo(GfxRenderer& r, int infoX, int infoY, int infoW,
                  const char* title, const char* author,
                  bool progressValid, int percentage,
                  int currentPage, int totalPages) {
  // Book title: bold, same body font size, up to six visual lines.
  // Each line is width-safe and truncated if necessary.
  constexpr int kTitleLines = 6;
  constexpr int kTitleLineGap = 20;
  constexpr int kAuthorGap = 8;
  constexpr int kProgressGap = 20;
  const auto titleLines = r.wrappedText(
      kInfoFont, title ? title : "", infoW, kTitleLines, EpdFontFamily::BOLD);

  int cursorY = infoY;
  for (const auto& line : titleLines) {
    const std::string safe =
        r.truncatedText(kInfoFont, line.c_str(), infoW, EpdFontFamily::BOLD);
    r.drawText(kInfoFont, infoX, cursorY, safe.c_str(), true,
               EpdFontFamily::BOLD);
    cursorY += kTitleLineGap;
  }
  if (author && *author) {
    cursorY += kAuthorGap;
    const std::string safeAuthor =
        r.truncatedText(kInfoFont, author, infoW, EpdFontFamily::REGULAR);
    r.drawText(kInfoFont, infoX, cursorY, safeAuthor.c_str(), true,
               EpdFontFamily::REGULAR);
  }

  // Progress starts 20px lower than the original V6 position.
  const int progressY = infoY + 128;
  r.drawText(kInfoFont, infoX, progressY, "Tiến trình đọc", true,
             EpdFontFamily::BOLD);

  if (!progressValid) return;
  const int pct = std::clamp(percentage, 0, 100);
  const int barY = progressY + kProgressGap;
  const int barW = infoW;
  const int barH = 8;
  r.drawRoundedRect(infoX, barY, barW, barH, 1, 4, true);
  const int fillW = std::max(1, (barW - 2) * pct / 100);
  r.fillRoundedRect(infoX + 1, barY + 1, fillW, barH - 2, 1, Color::Black);
  const std::string pctText = std::to_string(pct) + "%";
  const std::string pages =
      std::to_string(currentPage) + " / " + std::to_string(totalPages) + " trang";
  const int pctWidth =
      r.getTextWidth(kInfoFont, pctText.c_str(), EpdFontFamily::BOLD);
  const int pageWidth =
      r.getTextWidth(kInfoFont, pages.c_str(), EpdFontFamily::BOLD);
  const int metricsY = barY + 22;
  const int pageX = infoX + infoW - pageWidth;
  const int minGap = 8;
  if (pageWidth > 0 && pageWidth + pctWidth + minGap <= infoW) {
    r.drawText(kInfoFont, infoX, metricsY, pctText.c_str(), true,
               EpdFontFamily::BOLD);
    r.drawText(kInfoFont, pageX, metricsY, pages.c_str(), true,
               EpdFontFamily::BOLD);
  } else {
    r.drawText(kInfoFont, infoX, metricsY, pctText.c_str(), true,
               EpdFontFamily::BOLD);
    const int remainingWidth = std::max(1, infoW - pctWidth - minGap);
    const std::string safePages =
        r.truncatedText(kInfoFont, pages.c_str(), remainingWidth,
                        EpdFontFamily::BOLD);
    const int safePageWidth =
        r.getTextWidth(kInfoFont, safePages.c_str(), EpdFontFamily::BOLD);
    r.drawText(kInfoFont, infoX + infoW - safePageWidth, metricsY,
               safePages.c_str(), true, EpdFontFamily::BOLD);
  }
}
void drawBookCard(GfxRenderer& r, const HomeRenderContext& h) {
  const int x = 20;
  const int y = 320;
  const int w = r.getScreenWidth() - 40;
  const int height = 360;
  r.fillRoundedRect(x, y, w, height, kCardRadius, Color::White);
  r.drawRoundedRect(x, y, w, height, 2, kCardRadius, true);

  const int dividerX = 230;
  r.drawLine(dividerX, y + 16, dividerX, y + height - 16, 1, true);
  r.drawText(kHeadingFont, 34, y + 18, "ĐANG ĐỌC", true, EpdFontFamily::BOLD);
  r.drawText(kHeadingFont, 248, y + 18, "THÔNG TIN", true, EpdFontFamily::BOLD);

  const int coverX = 40;
  const int coverY = y + 55;
  const int coverW = 155;
  const int coverH = 300;
  const int infoX = 248;
  const int infoW = 192;
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
    drawBookInfo(r, infoX, y + 58, infoW,
                 book.title.c_str(), book.author.c_str(),
                 h.readingProgressValid, h.readingPercentage,
                 h.readingCurrentPage, h.readingTotalPages);

    r.drawLine(infoX, y + 270, infoX + infoW, y + 270, 1, true);
    drawOwner(r, infoX, y + 294, infoW, h);
    return;
  }
  drawDemoCover(r, coverX, coverY, coverW, coverH);
  drawBookInfo(r, infoX, y + 58, infoW,
               "The Hitchhiker's Guide to the Galaxy", "Douglas Adams",
               true, 62, 173, 278);

  r.drawLine(infoX, y + 270, infoX + infoW, y + 270, 1, true);
  drawOwner(r, infoX, y + 294, infoW, h);
}
// RoundedRaff Home menu is intentionally icon-only. HomeActivity still owns the
// dynamic menu model; this renderer only lays out 4/5/6 icons uniformly.
void drawHomeMenu(GfxRenderer& r, const HomeRenderContext& h) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int side = metrics.contentSidePadding;
  const int gap = metrics.menuSpacing;
  const int top =
      metrics.homeTopPadding + metrics.homeCoverTileHeight + metrics.homeMenuTopOffset;
  const int height =
      std::max(1, r.getScreenHeight() - top - metrics.buttonHintsHeight);
  const int count = std::max(1, h.menuCount);
  const int available = r.getScreenWidth() - side * 2;
  const int buttonW = std::max(1, (available - gap * (count - 1)) / count);
  for (int i = 0; i < h.menuCount; ++i) {
    const int bx = side + i * (buttonW + gap);
    const bool selected = h.selectorIndex == i;
    // Icon-only menu: keep the background white for every item so the
    // existing monochrome bitmap stays visible. Selection is indicated by
    // a heavier rounded border instead of inverting the whole button.
    r.fillRoundedRect(bx, top, buttonW, height, kMenuRadius, Color::White);
    r.drawRoundedRect(bx, top, buttonW, height,
                      selected ? 3 : 1, kMenuRadius, true);
    const uint8_t* icon = iconBitmap(h.rowIcon(i));
    if (icon) {
      // Icon-only Home menu: keep the icon large and optically centered.
      // The menu remains dynamic (4/5/6 buttons); only button width changes.
      constexpr int iconSize = 30;
      const int iconX = bx + (buttonW - iconSize) / 2;
      const int iconY = top + (height - iconSize) / 2;
      r.drawIcon(icon, iconX, iconY, iconSize);
    }
  }
}

}  // namespace

void RoundedRaffTheme::drawHeader(const GfxRenderer& renderer, Rect rect, const char* title,
                                  const char* subtitle) const {
  BaseTheme::drawHeader(renderer, rect, title, subtitle);
}

void RoundedRaffTheme::drawHome(GfxRenderer& renderer, const HomeRenderContext& home) const {
  // RoundedRaff Home is designed specifically for the original X4 logical
  // portrait framebuffer. Never apply its geometry to other panel sizes.
  if (renderer.getScreenWidth() != 480 || renderer.getScreenHeight() != 800) {
    BaseTheme::drawHome(renderer, home);
    return;
  }
  drawCalendar(renderer, home);
  drawBookCard(renderer, home);
  drawHomeMenu(renderer, home);
}
void RoundedRaffTheme::drawRecentBookCover(
    GfxRenderer& renderer, Rect rect, const std::vector<RecentBook>& recentBooks,
    const int selectorIndex, bool& coverRendered, bool& coverBufferStored,
    bool& bufferRestored, std::function<bool()> storeCoverBuffer) const {
  BaseTheme::drawRecentBookCover(renderer, rect, recentBooks, selectorIndex,
                                 coverRendered, coverBufferStored, bufferRestored,
                                 std::move(storeCoverBuffer));
}
int RoundedRaffTheme::getMenuRowHeight(const GfxRenderer& renderer) const {
  return renderer.getLineHeight(kHeadingFont) + 20;
}

void RoundedRaffTheme::drawButtonMenu(
    GfxRenderer& renderer, Rect rect, int buttonCount, int selectedIndex,
    const std::function<std::string(int index)>& buttonLabel,
    const std::function<UIIcon(int index)>& rowIcon) const {
  BaseTheme::drawButtonMenu(renderer, rect, buttonCount, selectedIndex,
                            buttonLabel, rowIcon);
}
void RoundedRaffTheme::drawTextField(const GfxRenderer& renderer, Rect rect,
                                     const int textWidth, bool cursorMode,
                                     int contentStartX, int contentWidth) const {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int lineHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int lineY = rect.y + rect.height + lineHeight + metrics.verticalSpacing;
  const int thickness = cursorMode ? 3 : 2;
  if (contentWidth > 0) {
    renderer.drawLine(rect.x + contentStartX, lineY,
                      rect.x + contentStartX + contentWidth - 1, lineY,
                      thickness, true);
    return;
  }
  constexpr int hPadding = 8;
  const int lineW = textWidth + hPadding * 2;
  const int lineStart = rect.x + (rect.width - lineW) / 2;
  renderer.drawLine(lineStart, lineY, lineStart + lineW - 1, lineY, thickness, true);
}

void RoundedRaffTheme::drawButtonHints(GfxRenderer& renderer, const char* btn1,
                                       const char* btn2, const char* btn3,
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
  const int textY =
      hintY + (hintHeight - renderer.getLineHeight(kGuideFontId)) / 2;
  const bool backDisabled = (btn1 == nullptr || btn1[0] == '\0');
  const int leftGroupX = sidePadding;
  const int rightGroupX = leftGroupX + groupWidth + groupGap;
  const std::string backLabel = backDisabled ? "" : std::string(btn1);
  const std::string selectText =
      (btn2 && btn2[0] != '\0') ? std::string(btn2) : "";
  const std::string upText =
      (btn3 && btn3[0] != '\0') ? std::string(btn3) : "";
  const std::string downText =
      (btn4 && btn4[0] != '\0') ? std::string(btn4) : "";
  renderer.drawRoundedRect(leftGroupX, hintY, groupWidth, hintHeight, 1, 14, true);
  renderer.drawRoundedRect(rightGroupX, hintY, groupWidth, hintHeight, 1, 14, true);
  constexpr int inner = 14;
  if (!backDisabled) {
    const std::string safeBack =
        renderer.truncatedText(kGuideFontId, backLabel.c_str(),
                               groupWidth - inner * 2, EpdFontFamily::REGULAR);
    renderer.drawText(kGuideFontId, leftGroupX + inner, textY,
                      safeBack.c_str(), true, EpdFontFamily::REGULAR);
  }
  const std::string safeSelect =
      renderer.truncatedText(kGuideFontId, selectText.c_str(),
                             groupWidth - inner * 2, EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, leftGroupX + groupWidth - inner -
                        renderer.getTextWidth(kGuideFontId, safeSelect.c_str(),
                                              EpdFontFamily::REGULAR),
                    textY, safeSelect.c_str(), true, EpdFontFamily::REGULAR);
  const std::string safeUp =
      renderer.truncatedText(kGuideFontId, upText.c_str(),
                             groupWidth - inner * 2, EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, rightGroupX + inner, textY,
                    safeUp.c_str(), true, EpdFontFamily::REGULAR);
  const std::string safeDown =
      renderer.truncatedText(kGuideFontId, downText.c_str(),
                             groupWidth - inner * 2, EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, rightGroupX + groupWidth - inner -
                        renderer.getTextWidth(kGuideFontId, safeDown.c_str(),
                                              EpdFontFamily::REGULAR),
                    textY, safeDown.c_str(), true, EpdFontFamily::REGULAR);
  renderer.setOrientation(origOrientation);
}
