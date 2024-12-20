/**
 * \file editor/content_browser_item.hpp
 **/
#ifndef OTHER_ENGINE_CONTENT_BROWSER_ITEM_HPP
#define OTHER_ENGINE_CONTENT_BROWSER_ITEM_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

#include "core/defines.hpp"
#include "core/directory.hpp"
#include "core/ref_counted.hpp"

#include "asset/asset_defines.hpp"
#include "asset/asset_registry.hpp"

#include "rendering/texture.hpp"

#include "editor/editor_images.hpp"

namespace other {

  enum class CBAction : uint16_t {
    IDENTITY = 0,
    REFRESH = bit(0),
    CLEAR_SELECTED = bit(1),
    SELECTED = bit(2),
    DESELECTED = bit(3),
    HOVERED = bit(4),
    RENAMED = bit(5),
    CONFIRM_DELETE = bit(6),
    DRAGGING = bit(7),
    MOVED = bit(8),
    OPEN_NATIVE = bit(9),
    OPEN_EXTERNAL = bit(10),
    RELOAD = bit(11),
    COPY = bit(12),
    DUPLICATE = bit(13),
    START_RENAMING = bit(14),
    ACTIVATED = bit(15),

    NUM_CB_ACTIONS,
    INVALID_CB_ACTION = NUM_CB_ACTIONS
  };

  struct CBActionResult {
    uint16_t field = 0;

    void Set(CBAction action, bool value) {
      if (value) {
        field |= ValOf(action);
      } else {
        field &= ~ValOf(action);
      }
    }

    bool Has(CBAction action) const {
      return field & ValOf(action);
    }
  };

  class ContentBrowserItem : public RefCounted {
   public:
    enum class Type {
      DIRECTORY,
      FILE,

      NUM_ITEM_TYPES,
      INVALID_ITEM_TYPE = NUM_ITEM_TYPES,
    };

    ContentBrowserItem(Type type, UUID file_handle, const std::string& name, const Ref<Texture>& icon)
        : type(type), file_handle(file_handle), filename(name), icon(icon) {
      std::ranges::fill(rename_buffer, 0);
    }
    virtual ~ContentBrowserItem() {}

    void OnRenderBegin();
    CBActionResult Render();
    void OnRenderEnd();

    virtual void Delete() {}
    virtual bool Move(const Path& new_path);

    Type GetType() const;
    UUID GetHandle() const;
    const std::string& GetFilename() const;
    const Ref<Texture>& GetIcon() const;

    bool IsSelected() const;
    void Select();
    void Deselect();
    void Rename(const std::string& name);
    void StartRenaming();

   protected:
    virtual void OnRenamed(const std::string& new_name);
    virtual void RenderCustomContextItems() {}
    virtual void UpdateDrop(CBActionResult& result) {}

    virtual std::string OverrideDisplayName(const std::string& new_name);

   private:
    Type type;
    UUID file_handle;
    std::string filename;
    std::string display_name;

    Ref<Texture> icon = nullptr;

    bool renaming = false;
    bool dragging = false;
    bool selected = false;
    bool just_selected = false;

    constexpr static size_t kMaxBufferSize = 256;
    std::array<char, kMaxBufferSize> rename_buffer;

    void OnContextMenuOpen(CBActionResult& result);
    void SetDisplayName();
  };

  class CBDirectory : public ContentBrowserItem {
   public:
    CBDirectory(const Ref<Directory>& directory);
    virtual ~CBDirectory() override {}

    Ref<Directory> directory = nullptr;
    std::vector<Ref<CBDirectory>> children;

    virtual void Delete() override;
    virtual bool Move(const Path& path) override;

   private:
    virtual void OnRenamed(const std::string& name) override;
    virtual void UpdateDrop(CBActionResult& result) override;

    void UpdateDirectoryPath(Ref<Directory> directory, const Path& new_parent, const Path& new_name);
  };

  class CBItem : public ContentBrowserItem {
   public:
    CBItem(const Ref<FileHandle>& asset_data, const Ref<Texture2D>& icon);
    virtual ~CBItem() override {}

    Ref<FileHandle> asset_data = nullptr;

    virtual void Delete() override;
    virtual bool Move(const Path& path) override;

   private:
    virtual void OnRenamed(const std::string& name) override;
    virtual std::string OverrideDisplayName(const std::string& new_name) override;
  };

  struct CBItemList {
    static constexpr size_t invalid_idx = std::numeric_limits<size_t>::max();

    std::vector<Ref<ContentBrowserItem>>::iterator begin() { return items.begin(); };
    std::vector<Ref<ContentBrowserItem>>::iterator end() { return items.end(); };
    std::vector<Ref<ContentBrowserItem>>::const_iterator begin() const { return items.begin(); };
    std::vector<Ref<ContentBrowserItem>>::const_iterator end() const { return items.end(); };

    Ref<ContentBrowserItem>& operator[](size_t idx) { return items[idx]; }
    const Ref<ContentBrowserItem>& operator[](size_t idx) const { return items[idx]; }

    size_t size();
    void clear();
    void erase(AssetHandle handle);
    size_t find(AssetHandle handle);
    void push_back(const Ref<ContentBrowserItem>& item);

   private:
    std::vector<Ref<ContentBrowserItem>> items;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_CONTENT_BROWSER_ITEM_HPP
