
#ifndef MOUSEEVENT_H
#define MOUSEEVENT_H
#include "Event.h"
#include "Base/MouseCodes.h"

namespace viz {
	namespace system {
		class mouseEvent : public event{
		public:
			virtual ~mouseEvent() = default;

			eventCategory getEventCat() const override {
				return eventCategory::eventCatMouse;
			}

			static eventType GetStaticType() { return eventType::noEvent;  }
		};

		class mouseEventPressed : public mouseEvent {
		public:
			mouse::mouseCodes getMousePressed() const { return mousePressed->mouseButton; }

			mouseEventPressed(const mouse::mouseCodes& mouse) { mousePressed = new eventData; mousePressed->mouseButton = mouse; }
			mouseEventPressed(const mouse::mouseCodes&& mouse) { mousePressed = new eventData; mousePressed->mouseButton = mouse; }

			static eventType GetStaticType() { return eventType::mouseButtonPress; }
			eventType getEventType() const override { return eventType::mouseButtonPress; }
			eventData* getData() const override { return mousePressed;  }

			std::string toString() const {
				std::stringstream ss;
				ss << "mouseEventPressed: " << getMousePressed();
				return ss.str();
			}

			~mouseEventPressed() { delete mousePressed; }
		private:
			eventData* mousePressed;
		};

		class mouseEventReleased : public mouseEvent {
		public:
			mouse::mouseCodes getMouseReleased() const { return mouseReleased->mouseButton; }

			static eventType GetStaticType() { return eventType::mouseButtonRelease; }
			eventType getEventType() const override { return eventType::mouseButtonRelease; }

			mouseEventReleased(const mouse::mouseCodes& mouse) { mouseReleased = new eventData; mouseReleased->mouseButton = mouse; }
			mouseEventReleased(const mouse::mouseCodes&& mouse) { mouseReleased = new eventData; mouseReleased->mouseButton = mouse; }
			eventData* getData() const override { return mouseReleased; }

			std::string toString() const {
				std::stringstream ss;
				ss << "mouseEventReleased: " << getMouseReleased();
				return ss.str();
			}

			~mouseEventReleased() { delete mouseReleased; }
		private:
			eventData* mouseReleased;
		};


		class mouseEventMoved : public mouseEvent {
		public:
			mouseEventMoved(const float&& x, const float&& y) {
				mousePos = new eventData;
				mousePos->floatPairedData = { x, y };
			}

			mouseEventMoved(const float& x, const float& y){
				mousePos = new eventData;
				mousePos->floatPairedData = { x, y };
			}

			static eventType GetStaticType() { return eventType::mouseMove; }
			eventType getEventType() const override { return eventType::mouseMove; }
			eventData* getData() const override { return mousePos; }
			

			std::string toString() const {
				std::stringstream ss;
				ss << "mouseEventMoved: (" << mousePos->floatPairedData.first << ", " << mousePos->floatPairedData.second << ")\n";
				return ss.str();
			}

			std::pair<float, float> getMousePos() {
				return mousePos->floatPairedData;
			}

			~mouseEventMoved() { delete mousePos;  }
		private:
			eventData* mousePos;
		};

		class mouseEventScrolled : public mouseEvent {
		public:
			mouseEventScrolled(const float&& xOffset, const float&& yOffset) {
				scrollOffset = new eventData;
				scrollOffset->floatPairedData = { xOffset, yOffset };
			}

			mouseEventScrolled(const float& xOffset, const float& yOffset) {
				scrollOffset = new eventData;
				scrollOffset->floatPairedData = { xOffset, yOffset };
			}

			static eventType GetStaticType() { return eventType::mouseScroll; }
			eventType getEventType() const override { return eventType::mouseScroll; }
			eventData* getData() const override { return scrollOffset; }


			std::string toString() const {
				std::stringstream ss;
				ss << "mouseEventScrolled: (" << scrollOffset->floatPairedData.first << ", " << scrollOffset->floatPairedData.second << ")\n";
				return ss.str();
			}

			std::pair<float, float> getMouseScrollOffset() {
				return scrollOffset->floatPairedData;
			}

			~mouseEventScrolled() { delete scrollOffset;  }
		private:
			eventData* scrollOffset;
		};
	}
}

#endif