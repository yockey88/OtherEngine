/**
 * \file thread/message.hpp
 **/
#ifndef OTHER_ENGINE_MESSAGE_HPP
#define OTHER_ENGINE_MESSAGE_HPP

#include <vector>
#include <concepts>
#include <cstdint>

#include "core/logger.hpp"
#include "core/defines.hpp"

namespace other {

  enum MessageType {
    INITIALIZE_MSG ,
    START_MSG ,
    READY_MSG ,

    NUM_MSG_TYPES ,
    INVALID_MSG_TYPE = NUM_MSG_TYPES 
  };

}

template<> 
struct fmt::formatter<other::MessageType> : public fmt::formatter<std::string_view> {
  auto format(other::MessageType type , fmt::format_context& ctx) {
    constexpr std::string_view fallback = "UNKNOWN Message Type";
    switch (type) {
      FMT_ENUM(MessageType::INITIALIZE_MSG);
      FMT_ENUM(MessageType::START_MSG);
      FMT_ENUM(MessageType::READY_MSG);
      default:
        return fmt::formatter<std::string_view>::format(fallback , ctx);
    }
  }
};

namespace other {

  struct MessageMetadata {
    MessageType type;
    uint32_t size;
  };

  class Message {
    public:
      template <typename T>
      Message(MessageMetadata metadata , T data) 
          : metadata(metadata) {
        uint8_t* data_bytes = reinterpret_cast<uint8_t*>(&data);
        for (size_t i = 0; i < sizeof(T); ++i) {
          bytes.push_back(data_bytes[i]);
        }
      }

      ~Message() {}

      MessageMetadata GetMeta() {
        return metadata;
      }

      template <typename T>
      T& Get() {
        return *reinterpret_cast<T*>(bytes.data());
      }

    private:
      MessageMetadata metadata;
      std::vector<uint8_t> bytes;
  };

  template <typename T>
    concept MsgType = std::same_as<T , Message> || std::derived_from<T , Message>;

  template <typename T , typename... Args>
    concept CanConstructFrom = std::constructible_from<T , Args...> || requires (T t , Args... args) {
      { T{args...} };
    };

  template <typename T , MsgType M , typename... Args>
    requires CanConstructFrom<T , Args...> && requires (T t , Args... args) {
      { t == T(args...) };
    }
  bool MsgEquals(const Scope<M>& message , MessageType e , Args&&... args) {
    if (message == nullptr) {
      OE_ERROR("Message is nullptr! (Checking against {})" , e);
      return false;
    }

    if (message->GetMeta().type != e) {
      OE_ERROR("Message type is not what was expected [{}] != [{}]" , message->GetMeta().type , e);
      return false;
    }
    
    return true;
  }

#pragma pack(push , 1)

  struct ReadyMessage {
    bool ready = false;

    constexpr auto operator<=>(const ReadyMessage& other) const = default;
  };

  struct InitializeMessage {
    constexpr auto operator<=>(const InitializeMessage& other) const = default;
  };

  struct StartMessage {
    constexpr auto operator<=>(const StartMessage& other) const = default;
  };

#pragma pack(pop)

} // namespace other

#endif // !OTHER_ENGINE_MESSAGE_HPP
