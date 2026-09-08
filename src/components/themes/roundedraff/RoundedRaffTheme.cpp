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
constexpr int kCoverRadius = 18;
constexpr int kCardRadius = 20;
constexpr int kMenuRadius = 16;
constexpr int kTitleFontId = UI_12_FONT_ID;
constexpr int kMenuFontId = UI_10_FONT_ID;
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

void drawCalendar(GfxRenderer& r, const HomeRenderContext& h) {
  const int center = r.getScreenWidth() / 2;
  r.drawText(kTitleFontId, center - r.getTextWidth(kTitleFontId, h.calendarMonth, EpdFontFamily::BOLD) / 2,
             h.headerRect.y + 14, h.calendarMonth, true, EpdFontFamily::BOLD);
  const int dayY = h.headerRect.y + 43;
  const int dayW = r.getTextWidth(NOTOSANS_18_FONT_ID, h.calendarDay, EpdFontFamily::BOLD);
  r.drawText(NOTOSANS_18_FONT_ID, center - dayW / 2, dayY, h.calendarDay, true, EpdFontFamily::BOLD);
  const int weekdayY = h.headerRect.y + 101;
  const int weekdayW = r.getTextWidth(kTitleFontId, h.calendarWeekday, EpdFontFamily::BOLD);
  r.drawText(kTitleFontId, center - weekdayW / 2, weekdayY, h.calendarWeekday, true, EpdFontFamily::BOLD);
  const int lunarW = r.getTextWidth(kMenuFontId, h.calendarLunar);
  r.drawText(kMenuFontId, center - lunarW / 2, h.headerRect.y + 132, h.calendarLunar, true);
}

void drawOwner(GfxRenderer& r, int x, int y, int width) {
  constexpr const char* kOwnerTitle = "CHỦ MÁY";
  constexpr const char* kOwnerName = "Bs Phương";
  constexpr const char* kOwnerPhone = "0843191075";
  constexpr const char* kOwnerEmail = "nhakhoaphuong@gmail.com";
  r.drawText(kMenuFontId, x, y, kOwnerTitle, true, EpdFontFamily::BOLD);
  r.drawText(kMenuFontId, x, y + 22, kOwnerName, true);
  r.drawText(kMenuFontId, x, y + 42, kOwnerPhone, true);
  const std::string email = r.truncatedText(kMenuFontId, kOwnerEmail, width);
  r.drawText(kMenuFontId, x, y + 62, email.c_str(), true);
}

void drawBookCard(GfxRenderer& r, const HomeRenderContext& h) {
  const Rect card = h.coverRect;
  const int x = RoundedRaffMetrics::values.contentSidePadding;
  const int w = card.width - 2 * RoundedRaffMetrics::values.contentSidePadding;
  const int headingY = card.y + 18;
  const int bodyY = card.y + 54;
  const int infoX = card.x + 190;
  const int infoRight = card.x + card.width - RoundedRaffMetrics::values.contentSidePadding;
  const int infoW = std::max(1, infoRight - infoX);
  const int coverX = card.x + 20;
  const int coverW = std::max(1, infoX - coverX - 14);
  const int coverH = RoundedRaffMetrics::values.homeCoverHeight;
  const int coverY = bodyY;

  r.fillRoundedRect(x, card.y, w, card.height, kCardRadius, Color::White);

  r.drawText(kTitleFontId, x + 12, headingY, "ĐANG ĐỌC", true, EpdFontFamily::BOLD);
  r.drawText(kTitleFontId, infoX, headingY, "THÔNG TIN", true, EpdFontFamily::BOLD);

  if (h.recentBooks.empty()) {
    r.drawText(kMenuFontId, x + 12, bodyY + 40, "Chưa có sách đang đọc", true);
    r.drawText(kMenuFontId, x + 12, bodyY + 64, "Mở một cuốn sách để bắt đầu", true);
    drawOwner(r, infoX, bodyY + 24, infoW);
    return;
  }

  const RecentBook& book = h.recentBooks.front();
  bool hasCover = false;
  int actualCoverW = coverW;
  if (!book.coverBmpPath.empty()) {
    const std::string path = UITheme::getCoverThumbPath(book.coverBmpPath, coverH);
    HalFile file;
    if (Storage.openFileForRead("HOME", path, file)) {
      Bitmap bitmap(file);
      if (bitmap.parseHeaders() == BmpReaderError::Ok) {
        actualCoverW = std::min(bitmap.getWidth(), coverW);
        const int cx = coverX + (coverW - actualCoverW) / 2;
        r.drawBitmap(bitmap, cx, coverY, actualCoverW, coverH);
        r.maskRoundedRectOutsideCorners(cx, coverY, actualCoverW, coverH, kCoverRadius, Color::White);
        hasCover = true;
      }
      file.close();
    }
  }
  if (!hasCover) {
    r.fillRoundedRect(coverX, coverY, coverW, coverH, kCoverRadius, Color::LightGray);
    r.drawIcon(CoverIcon, coverX + std::max(0, (coverW - 32) / 2),
               coverY + std::max(0, (coverH - 32) / 2), 32);
  }

  const auto titleLines = r.wrappedText(kTitleFontId, book.title.c_str(), infoW, 2, EpdFontFamily::BOLD);
  int y = bodyY + 6;
  for (const auto& line : titleLines) {
    r.drawText(kTitleFontId, infoX, y, line.c_str(), true, EpdFontFamily::BOLD);
    y += r.getLineHeight(kTitleFontId);
  }
  if (!book.author.empty()) {
    y += 4;
    const std::string author = r.truncatedText(kMenuFontId, book.author.c_str(), infoW);
    r.drawText(kMenuFontId, infoX, y, author.c_str(), true);
    y += r.getLineHeight(kMenuFontId) + 12;
  } else {
    y += 12;
  }

  r.drawText(kMenuFontId, infoX, y, "Tiến trình đọc", true, EpdFontFamily::BOLD);
  y += 20;
  if (h.readingProgressValid) {
    const int barW = infoW;
    const int barH = 8;
    r.drawRoundedRect(infoX, y, barW, barH, 1, kCardRadius, true);
    const int fill = std::max(1, (barW - 2) * std::clamp(h.readingPercentage, 0, 100) / 100);
    r.fillRoundedRect(infoX + 1, y + 1, fill, barH - 2, 1, Color::Black);
    y += 20;
    const std::string pct = std::to_string(h.readingPercentage) + "%";
    r.drawText(kMenuFontId, infoX, y, pct.c_str(), true, EpdFontFamily::BOLD);
    const std::string pages = std::to_string(h.readingCurrentPage) + " / " + std::to_string(h.readingTotalPages) + " trang";
    const int pw = r.getTextWidth(kMenuFontId, pages.c_str());
    r.drawText(kMenuFontId, infoRight - pw, y, pages.c_str(), true);
    y += 28;
  } else {
    r.drawText(kMenuFontId, infoX, y, "Chưa có dữ liệu tiến trình", true);
    y += 28;
  }

  drawOwner(r, infoX, std::min(y + 18, card.y + card.height - 88), infoW);
}

}  // namespace

void RoundedRaffTheme::drawHeader(const GfxRenderer& renderer, Rect rect, const char* title,
                                  const char* subtitle) const {
  // Normal activity headers remain untouched. Home uses drawHome().
  BaseTheme::drawHeader(renderer, rect, title, subtitle);
}

void RoundedRaffTheme::drawHome(GfxRenderer& renderer, const HomeRenderContext& home) const {
  drawCalendar(renderer, home);
  if (home.recentBooks.empty() || !home.bufferRestored || !home.coverRendered) {
    drawBookCard(renderer, home);
    if (!home.recentBooks.empty()) {
      home.coverBufferStored = home.storeCoverBuffer();
      home.coverRendered = home.coverBufferStored;
    }
  }

  const int sidePadding = RoundedRaffMetrics::values.contentSidePadding;
  const int gap = RoundedRaffMetrics::values.menuSpacing;
  const int count = std::max(1, home.menuCount);
  const int availableWidth = std::max(0, home.menuRect.width - sidePadding * 2);
  const int buttonWidth = std::max(1, (availableWidth - gap * (count - 1)) / count);
  const int buttonHeight = std::min(72, std::max(1, home.menuRect.height));
  const int y = home.menuRect.y + std::max(0, (home.menuRect.height - buttonHeight) / 2);

  for (int i = 0; i < home.menuCount; ++i) {
    const int bx = sidePadding + i * (buttonWidth + gap);
    const bool selected = home.selectorIndex == i;
    renderer.fillRoundedRect(bx, y, buttonWidth, buttonHeight, kMenuRadius, selected ? Color::Black : Color::White);

    const uint8_t* icon = iconBitmap(home.rowIcon(i));
    const int iconSize = buttonWidth >= 74 ? 20 : 18;
    const int iconY = y + 10;
    if (icon) {
      renderer.drawIcon(icon, bx + (buttonWidth - iconSize) / 2, iconY, iconSize);
    }

    const std::string label = home.menuLabel(i);
    const int textWidth = std::max(1, buttonWidth - 8);
    const auto lines = renderer.wrappedText(kMenuFontId, label.c_str(), textWidth, 2, EpdFontFamily::BOLD);
    const int lineHeight = renderer.getLineHeight(kMenuFontId);
    const int textTop = icon ? y + 31 : y + (buttonHeight - static_cast<int>(lines.size()) * lineHeight) / 2;
    for (int line = 0; line < static_cast<int>(lines.size()); ++line) {
      const int tw = renderer.getTextWidth(kMenuFontId, lines[line].c_str(), EpdFontFamily::BOLD);
      renderer.drawText(kMenuFontId, bx + (buttonWidth - tw) / 2, textTop + line * lineHeight, lines[line].c_str(),
                        !selected, EpdFontFamily::BOLD);
    }
  }
}

void RoundedRaffTheme::drawRecentBookCover(GfxRenderer& renderer, Rect rect, const std::vector<RecentBook>& recentBooks,
                                           const int selectorIndex, bool& coverRendered, bool& coverBufferStored,
                                           bool& bufferRestored, std::function<bool()> storeCoverBuffer) const {
  BaseTheme::drawRecentBookCover(renderer, rect, recentBooks, selectorIndex, coverRendered, coverBufferStored,
                                 bufferRestored, std::move(storeCoverBuffer));
}

int RoundedRaffTheme::getMenuRowHeight(const GfxRenderer& renderer) const {
  return renderer.getLineHeight(kTitleFontId) + 20;
}

void RoundedRaffTheme::drawButtonMenu(GfxRenderer& renderer, Rect rect, int buttonCount, int selectedIndex,
                                      const std::function<std::string(int index)>& buttonLabel,
                                      const std::function<UIIcon(int index)>& rowIcon) const {
  // Kept for non-Home callers; Home itself uses drawHome().
  BaseTheme::drawButtonMenu(renderer, rect, buttonCount, selectedIndex, buttonLabel, rowIcon);
}

void RoundedRaffTheme::drawTextField(const GfxRenderer& renderer, Rect rect, const int textWidth, bool cursorMode,
                                     int contentStartX, int contentWidth) const {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int lineHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int lineY = rect.y + rect.height + lineHeight + metrics.verticalSpacing;
  const int thickness = cursorMode ? 3 : 2;

  if (contentWidth > 0) {
    renderer.drawLine(rect.x + contentStartX, lineY, rect.x + contentStartX + contentWidth - 1, lineY, thickness, true);
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
  const int bottomMargin = 10;
  const int hintHeight = RoundedRaffMetrics::values.buttonHintsHeight - 10;
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
  renderer.fillRect(leftGroupX, hintY, groupWidth, hintHeight, false);
  renderer.fillRect(rightGroupX, hintY, groupWidth, hintHeight, false);
  renderer.drawRoundedRect(leftGroupX, hintY, groupWidth, hintHeight, 2, 15, true);
  renderer.drawRoundedRect(rightGroupX, hintY, groupWidth, hintHeight, 2, 15, true);
  constexpr int inner = 16;
  const int selectWidth = renderer.getTextWidth(kGuideFontId, selectText.c_str(), EpdFontFamily::REGULAR);
  const int downWidth = renderer.getTextWidth(kGuideFontId, downText.c_str(), EpdFontFamily::REGULAR);
  if (!backDisabled) renderer.drawText(kGuideFontId, leftGroupX + inner, textY, backLabel.c_str(), true, EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, leftGroupX + groupWidth - inner - selectWidth, textY, selectText.c_str(), true,
                    EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, rightGroupX + inner, textY, upText.c_str(), true, EpdFontFamily::REGULAR);
  renderer.drawText(kGuideFontId, rightGroupX + groupWidth - inner - downWidth, textY, downText.c_str(), true,
                    EpdFontFamily::REGULAR);
  renderer.setOrientation(origOrientation);
}
