import requests
import time
import os
import json

BASE_URL = "http://localhost:8080"
LAYOUT_FILE = "1.png"


def wait_for_backend():
    print("Waiting for backend to start...")
    for _ in range(30):
        try:
            requests.get(f"{BASE_URL}/api/settings")
            print("Backend is up!")
            return True
        except requests.ConnectionError:
            time.sleep(1)
    print("Backend failed to start.")
    return False


def test_flow():
    if not wait_for_backend():
        return

    # 1. Create Event
    print("\n--- 1. Creating Event ---")
    event_data = {
        "name": "Test Layout Event",
        "location": "Lab",
        "eventDate": "2026-01-26",
    }
    res = requests.post(f"{BASE_URL}/api/events", json=event_data)
    if res.status_code != 201:
        print(f"Failed to create event: {res.text}")
        return

    event = res.json()["data"]
    event_id = event["id"]
    print(f"Event created: ID {event_id}")

    # 2. Upload Layout
    print(f"\n--- 2. Uploading Layout ({LAYOUT_FILE}) ---")
    if not os.path.exists(LAYOUT_FILE):
        print(f"Error: {LAYOUT_FILE} not found!")
        return

    with open(LAYOUT_FILE, "rb") as f:
        files = {"layout": (LAYOUT_FILE, f, "image/png")}
        res = requests.post(f"{BASE_URL}/api/events/{event_id}/layout", files=files)

    if res.status_code != 200:
        print(f"Failed to upload layout: {res.text}")
        return

    layout_data = res.json()["data"]
    print("Layout analyzed successfully!")
    print(f"Photo Count: {layout_data['photoCount']}")
    print(f"Slots found: {len(layout_data['slots'])}")
    for slot in layout_data["slots"]:
        print(
            f"  Slot {slot['id']}: x={slot['x']}, y={slot['y']}, w={slot['width']}, h={slot['height']}"
        )

    # 3. Verify Persistence (Get Config)
    print("\n--- 3. Verifying Persistence ---")
    res = requests.get(f"{BASE_URL}/api/events/{event_id}/config")
    if res.status_code != 200:
        print(f"Failed to get config: {res.text}")
        return

    config = res.json()["data"]
    if "layoutSlots" in config and config["layoutSlots"]:
        print("Success! Layout slots retrieved from config.")
        frames = config["layoutSlots"]["slots"]
        print(f"Retrieved {len(frames)} slots from config.")
    else:
        print("Error: Layout slots NOT found in config response.")


if __name__ == "__main__":
    test_flow()
