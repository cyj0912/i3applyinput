/*
The purpose of this program is to apply the following settings using XInput

TODO: rerun `gsettings list-recursively org.gnome.desktop.peripherals.touchpad`
under GNOME org.gnome.desktop.peripherals.touchpad tap-button-map 'default'
org.gnome.desktop.peripherals.touchpad click-method 'fingers'
org.gnome.desktop.peripherals.touchpad edge-scrolling-enabled false
org.gnome.desktop.peripherals.touchpad disable-while-typing true
org.gnome.desktop.peripherals.touchpad two-finger-scrolling-enabled true
org.gnome.desktop.peripherals.touchpad send-events 'enabled'
org.gnome.desktop.peripherals.touchpad speed 0.0
org.gnome.desktop.peripherals.touchpad scroll-method 'two-finger-scrolling'
org.gnome.desktop.peripherals.touchpad natural-scroll true
org.gnome.desktop.peripherals.touchpad middle-click-emulation false
org.gnome.desktop.peripherals.touchpad left-handed 'mouse'
org.gnome.desktop.peripherals.touchpad tap-to-click false
org.gnome.desktop.peripherals.touchpad tap-and-drag-lock false
org.gnome.desktop.peripherals.touchpad tap-and-drag true

gsd-mouse is no longer included in gnome starting from 3.??, becuase mutter now
manages all input devices. This is why this utility is needed when I run i3 in
isolation.

Reference:
https://gitlab.gnome.org/GNOME/mutter/-/blob/main/src/backends/meta-input-settings.c
  This is how GNOME does it. We don't use Xlib here; we use XCB instead.
*/

#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xinput.h>

xcb_generic_error_t *err;
xcb_connection_t *conn;

#define TP_LEN 32
uint8_t touchpad_ids[TP_LEN];
size_t touchpad_count = 0;

void change_property(size_t tp_index, const char *property, xcb_atom_t type,
                     int format, const void *data, unsigned nitems) {
  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(conn, 1, strlen(property), property);
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, &err);
  if (err) {
    printf("Failed to apply %s, could not retrieve atom for property\n",
           property);
    return;
  }
  xcb_atom_t prop_atom = reply->atom;
  xcb_void_cookie_t co = xcb_input_xi_change_property(
      conn, touchpad_ids[tp_index], XIPropModeReplace, format, prop_atom, type,
      nitems, data);
  err = xcb_request_check(conn, co);
  if (err) {
    printf("Failed to update property\n");
    return;
  }
}

int main(int argc, char **argv) {
  conn = xcb_connect(NULL, NULL);
  xcb_input_list_input_devices_cookie_t cookie =
      xcb_input_list_input_devices(conn);
  xcb_input_list_input_devices_reply_t *reply =
      xcb_input_list_input_devices_reply(conn, cookie, &err);
  xcb_input_device_info_iterator_t diter =
      xcb_input_list_input_devices_devices_iterator(reply);
  while (diter.rem) {
    xcb_get_atom_name_cookie_t acookie =
        xcb_get_atom_name(conn, diter.data->device_type);
    xcb_get_atom_name_reply_t *areply =
        xcb_get_atom_name_reply(conn, acookie, &err);
    char *type_name = "N/A";
    if (!err)
      type_name = xcb_get_atom_name_name(areply);
    printf("Dev type %d (%s), id %d, nclass %d, use %d\n",
           diter.data->device_type, type_name, diter.data->device_id,
           diter.data->num_class_info, diter.data->device_use);

    if (strcmp(type_name, "TOUCHPAD1") == 0) {
      printf("\tFound a touchpad\n");
      touchpad_ids[touchpad_count++] = diter.data->device_id;
    }

    xcb_input_device_info_next(&diter);
  }

  for (int i = 0; i < touchpad_count; i++) {
    char value = 1;
    change_property(0, "libinput Natural Scrolling Enabled", XA_INTEGER, 8,
                    &value, 1);
    change_property(0, "libinput Tapping Enabled", XA_INTEGER, 8, &value, 1);
  }

  xcb_disconnect(conn);
  return 0;
}
