/* Example for use of GNU gettext.
   This file is in the public domain.

   Source code of the C program.  */


/* Get GTK declarations.  */
#include <gtk/gtk.h>
#include <glib/gi18n.h>

/* Get getpid() declaration.  */
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

static void
quit_callback (GtkWidget *widget, void *data)
{
  g_application_quit (G_APPLICATION (data));
}

/* Forward declaration of GObject types.  */
#define HELLO_TYPE_APPLICATION_WINDOW (hello_application_window_get_type ())
#define HELLO_APPLICATION_WINDOW(obj)                           \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                           \
                               HELLO_TYPE_APPLICATION_WINDOW,   \
                               HelloApplicationWindow))

typedef struct _HelloApplicationWindow HelloApplicationWindow;
typedef struct _HelloApplicationWindowClass HelloApplicationWindowClass;

#define HELLO_TYPE_APPLICATION (hello_application_get_type ())
#define HELLO_APPLICATION(obj)                          \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),                   \
                               HELLO_TYPE_APPLICATION,  \
                               HelloApplication))

typedef struct _HelloApplication HelloApplication;
typedef struct _HelloApplicationClass HelloApplicationClass;

/* Custom application window implementation.  */

struct _HelloApplicationWindow
{
  GtkApplicationWindow parent;
  GtkWidget *label2;
  GtkWidget *button;
};

struct _HelloApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE (HelloApplicationWindow, hello_application_window,
               GTK_TYPE_APPLICATION_WINDOW);

static void
hello_application_window_init (HelloApplicationWindow *window)
{
  char *label;

  gtk_widget_init_template (GTK_WIDGET (window));
  label = g_strdup_printf (_("This program is running as process number %d."),
                           getpid ());
  gtk_label_set_label (GTK_LABEL (window->label2), label);
  g_free (label);
}

static void
hello_application_window_class_init (HelloApplicationWindowClass *klass)
{
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                               "/org/gnu/gettext/examples/hello/hello.ui");
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass),
                                        HelloApplicationWindow, label2);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass),
                                        HelloApplicationWindow, button);
}

static HelloApplicationWindow *
hello_application_window_new (HelloApplication *application)
{
  return g_object_new (HELLO_TYPE_APPLICATION_WINDOW,
                       "application", application,
                       NULL);
}

/* Custom application implementation.  */

struct _HelloApplication
{
  GtkApplication parent;
};

struct _HelloApplicationClass
{
  GtkApplicationClass parent_class;
};

G_DEFINE_TYPE (HelloApplication, hello_application, GTK_TYPE_APPLICATION);

static void
hello_application_init (HelloApplication *application)
{
}

static void
hello_application_activate (GApplication *application)
{
  HelloApplicationWindow *window;

  window = hello_application_window_new (HELLO_APPLICATION (application));
  g_signal_connect (window->button, "clicked",
                    G_CALLBACK (quit_callback), application);
  gtk_window_present (GTK_WINDOW (window));
}

static void
hello_application_class_init (HelloApplicationClass *klass)
{
  G_APPLICATION_CLASS (klass)->activate = hello_application_activate;
}

static HelloApplication *
hello_application_new (void)
{
  return g_object_new (HELLO_TYPE_APPLICATION,
                       "application-id", "org.gnu.gettext-examples.hello",
                       NULL);
}

int
main (int argc, char *argv[])
{
  /* Initializations.  */
  textdomain ("hello-c-gnome3");
  bindtextdomain ("hello-c-gnome3", LOCALEDIR);

  /* Start the application.  */
  return g_application_run (G_APPLICATION (hello_application_new ()),
                            argc, argv);
}
