/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sdljava.x.swig;

public interface SWIG_SDLEventConstants {
  public final static int SDL_NOEVENT = 0;
  public final static int SDL_ACTIVEEVENT = SDL_NOEVENT + 1;
  public final static int SDL_KEYDOWN = SDL_ACTIVEEVENT + 1;
  public final static int SDL_KEYUP = SDL_KEYDOWN + 1;
  public final static int SDL_MOUSEMOTION = SDL_KEYUP + 1;
  public final static int SDL_MOUSEBUTTONDOWN = SDL_MOUSEMOTION + 1;
  public final static int SDL_MOUSEBUTTONUP = SDL_MOUSEBUTTONDOWN + 1;
  public final static int SDL_JOYAXISMOTION = SDL_MOUSEBUTTONUP + 1;
  public final static int SDL_JOYBALLMOTION = SDL_JOYAXISMOTION + 1;
  public final static int SDL_JOYHATMOTION = SDL_JOYBALLMOTION + 1;
  public final static int SDL_JOYBUTTONDOWN = SDL_JOYHATMOTION + 1;
  public final static int SDL_JOYBUTTONUP = SDL_JOYBUTTONDOWN + 1;
  public final static int SDL_QUIT = SDL_JOYBUTTONUP + 1;
  public final static int SDL_SYSWMEVENT = SDL_QUIT + 1;
  public final static int SDL_EVENT_RESERVEDA = SDL_SYSWMEVENT + 1;
  public final static int SDL_EVENT_RESERVEDB = SDL_EVENT_RESERVEDA + 1;
  public final static int SDL_VIDEORESIZE = SDL_EVENT_RESERVEDB + 1;
  public final static int SDL_VIDEOEXPOSE = SDL_VIDEORESIZE + 1;
  public final static int SDL_EVENT_RESERVED2 = SDL_VIDEOEXPOSE + 1;
  public final static int SDL_EVENT_RESERVED3 = SDL_EVENT_RESERVED2 + 1;
  public final static int SDL_EVENT_RESERVED4 = SDL_EVENT_RESERVED3 + 1;
  public final static int SDL_EVENT_RESERVED5 = SDL_EVENT_RESERVED4 + 1;
  public final static int SDL_EVENT_RESERVED6 = SDL_EVENT_RESERVED5 + 1;
  public final static int SDL_EVENT_RESERVED7 = SDL_EVENT_RESERVED6 + 1;
  public final static int SDL_USEREVENT = 24;
  public final static int SDL_NUMEVENTS = 32;

  public final static int SDL_ACTIVEEVENTMASK = (1<<(SDL_ACTIVEEVENT));
  public final static int SDL_KEYDOWNMASK = (1<<(SDL_KEYDOWN));
  public final static int SDL_KEYUPMASK = (1<<(SDL_KEYUP));
  public final static int SDL_MOUSEMOTIONMASK = (1<<(SDL_MOUSEMOTION));
  public final static int SDL_MOUSEBUTTONDOWNMASK = (1<<(SDL_MOUSEBUTTONDOWN));
  public final static int SDL_MOUSEBUTTONUPMASK = (1<<(SDL_MOUSEBUTTONUP));
  public final static int SDL_MOUSEEVENTMASK = (1<<(SDL_MOUSEMOTION))|(1<<(SDL_MOUSEBUTTONDOWN))|(1<<(SDL_MOUSEBUTTONUP));
  public final static int SDL_JOYAXISMOTIONMASK = (1<<(SDL_JOYAXISMOTION));
  public final static int SDL_JOYBALLMOTIONMASK = (1<<(SDL_JOYBALLMOTION));
  public final static int SDL_JOYHATMOTIONMASK = (1<<(SDL_JOYHATMOTION));
  public final static int SDL_JOYBUTTONDOWNMASK = (1<<(SDL_JOYBUTTONDOWN));
  public final static int SDL_JOYBUTTONUPMASK = (1<<(SDL_JOYBUTTONUP));
  public final static int SDL_JOYEVENTMASK = (1<<(SDL_JOYAXISMOTION))|(1<<(SDL_JOYBALLMOTION))|(1<<(SDL_JOYHATMOTION))|(1<<(SDL_JOYBUTTONDOWN))|(1<<(SDL_JOYBUTTONUP));
  public final static int SDL_VIDEORESIZEMASK = (1<<(SDL_VIDEORESIZE));
  public final static int SDL_VIDEOEXPOSEMASK = (1<<(SDL_VIDEOEXPOSE));
  public final static int SDL_QUITMASK = (1<<(SDL_QUIT));
  public final static int SDL_SYSWMEVENTMASK = (1<<(SDL_SYSWMEVENT));

  public final static int SDL_ALLEVENTS = 0xFFFFFFFF;
  public final static int SDL_APPMOUSEFOCUS = 0x01;
  public final static int SDL_APPINPUTFOCUS = 0x02;
  public final static int SDL_APPACTIVE = 0x04;
  public final static int SDL_DEFAULT_REPEAT_DELAY = 500;
  public final static int SDL_DEFAULT_REPEAT_INTERVAL = 30;
  public final static int SDL_BUTTON_LEFT = 1;
  public final static int SDL_BUTTON_MIDDLE = 2;
  public final static int SDL_BUTTON_RIGHT = 3;
  public final static int SDL_BUTTON_WHEELUP = 4;
  public final static int SDL_BUTTON_WHEELDOWN = 5;
}
