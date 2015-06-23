import java.util.Vector;
import java.util.List;
import java.util.Iterator;
import java.util.Arrays;
import java.io.File;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.PrintWriter;
import java.io.IOException;
// JDOM classes used for document representation
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.Attribute;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

/**
 * Converts the matrix.xml file to a matrix.texi file, suitable for
 * being included from gettext's nls.texi.
 *
 * @author Bruno Haible
 */
public class Matrix {

  public static class PoFile {
    String domain;
    String team;
    int percentage;
    public PoFile (String domain, String team, int percentage) {
      this.domain = domain;
      this.team = team;
      this.percentage = percentage;
    }
  }

  public static class Data {
    List /* of String */ domains = new Vector();
    List /* of String */ teams = new Vector();
    List /* of PoFile */ po_files = new Vector();
  }

  public static final int FALSE = 0;
  public static final int TRUE = 1;
  public static final int EXTERNAL = 2;

  public static void spaces (PrintWriter stream, int n) {
    for (int i = n; i > 0; i--)
      stream.print(' ');
  }

  public static void main (String[] args) {
    Data data = new Data();

    SAXBuilder builder = new SAXBuilder(/*true*/); // "true" turns on validation
    Document doc;
    try {
      doc = builder.build(new File("matrix.xml"));
    } catch (IOException e) {
      e.printStackTrace();
      doc = null;
      System.exit(1);
    } catch (JDOMException e) {
      e.printStackTrace();
      doc = null;
      System.exit(1);
    }
    Element po_inventory = doc.getRootElement();
    {
      Element domains = po_inventory.getChild("domains");
      Iterator i = domains.getChildren("domain").iterator();
      while (i.hasNext()) {
        Element domain = (Element)i.next();
        data.domains.add(domain.getAttribute("name").getValue());
      }
    }
    {
      Element teams = po_inventory.getChild("teams");
      Iterator i = teams.getChildren("team").iterator();
      while (i.hasNext()) {
        Element team = (Element)i.next();
        data.teams.add(team.getAttribute("name").getValue());
      }
    }
    {
      Element po_files = po_inventory.getChild("PoFiles");
      Iterator i = po_files.getChildren("po").iterator();
      while (i.hasNext()) {
        Element po = (Element)i.next();
        String value = po.getText();
        data.po_files.add(
            new PoFile(
                po.getAttribute("domain").getValue(),
                po.getAttribute("team").getValue(),
                value.equals("") ? -1 : Integer.parseInt(value)));
      }
    }

    // Special treatment of clisp. The percentages are incorrect.
    if (!data.teams.contains("en"))
      data.teams.add("en");
    data.po_files.add(new PoFile("clisp","en",100));
    data.po_files.add(new PoFile("clisp","de",83));
    data.po_files.add(new PoFile("clisp","fr",58));
    data.po_files.add(new PoFile("clisp","es",54));
    data.po_files.add(new PoFile("clisp","nl",57));
    data.po_files.add(new PoFile("clisp","ru",74));

    // Obsolete domains.
    data.domains.remove("gettext");
    for (Iterator i = data.po_files.iterator(); i.hasNext(); ) {
      PoFile po = (PoFile)i.next();
      if (po.domain.equals("gettext"))
        i.remove();
    }

    try {
      FileWriter f = new FileWriter("matrix.texi");
      BufferedWriter bf = new BufferedWriter(f);
      PrintWriter stream = new PrintWriter(bf);

      String[] domains = (String[])data.domains.toArray(new String[0]);
      Arrays.sort(domains);
      String[] teams = (String[])data.teams.toArray(new String[0]);
      Arrays.sort(teams);
      int ndomains = domains.length;
      int nteams = teams.length;

      int[][] matrix = new int[ndomains][];
      for (int d = 0; d < ndomains; d++)
        matrix[d] = new int[nteams];
      int[] total_per_domain = new int[ndomains];
      int[] total_per_team = new int[nteams];
      int total = 0;
      {
        Iterator i = data.po_files.iterator();
        while (i.hasNext()) {
          PoFile po = (PoFile)i.next();
          if (po.percentage >= 50) {
            int d = Arrays.binarySearch(domains,po.domain);
            if (d < 0)
              throw new Error("didn't find domain \""+po.domain+"\"");
            int t = Arrays.binarySearch(teams,po.team);
            if (t < 0)
              throw new Error("didn't find team \""+po.team+"\"");
            matrix[d][t] = TRUE;
            total_per_domain[d]++;
            total_per_team[t]++;
            total++;
          } else if (po.percentage < 0) {
            int d = Arrays.binarySearch(domains,po.domain);
            if (d < 0)
              throw new Error("didn't find domain \""+po.domain+"\"");
            int t = Arrays.binarySearch(teams,po.team);
            if (t < 0) {
              System.err.println(po.domain+": didn't find team \""+po.team+"\"");
              continue;
            }
            matrix[d][t] = EXTERNAL;
          }
        }
      }

      int[] columnwidth = new int[nteams];
      for (int t = 0; t < nteams; t++)
        columnwidth[t] =
          Math.max(teams[t].length(),
                   Integer.toString(total_per_team[t]).length());

      // Split into separate tables, to keep 80 column width.
      int maxwidth = 80 - 21 - 5 - Integer.toString(total).length();

      // First determine how many groups are needed.
      int ngroups = 0;
      {
        int width = 0, last_team = 0;
        for (int t = 0; t < nteams; t++) {
          int newwidth = width + columnwidth[t] + 1;
          if (newwidth > maxwidth) {
            last_team = t;
            ngroups++;
            width = 0;
          }
          width += columnwidth[t] + 1;
        }
        if (last_team < nteams)
          ngroups++;
      }

      // Then initialize the size of each group.
      int[][] groups = new int[ngroups][];
      {
        int width = 0, last_team = 0, index = 0;
        for (int t = 0; t < nteams; t++) {
          int newwidth = width + columnwidth[t] + 1;
          if (newwidth > maxwidth) {
            groups[index++] = new int[] { last_team, t };
            last_team = t;
            width = 0;
          }
          width += columnwidth[t] + 1;
        }
        if (last_team < nteams)
          groups[index] = new int[] { last_team, nteams };
      }

      stream.println("@example");
      for (int group = 0; group < ngroups; group++) {
        if (group > 0)
          stream.println();

        stream.println("@group");

        if (group == 0)
          stream.print("Ready PO files      ");
        else
          stream.print("                    ");
        for (int t = groups[group][0]; t < groups[group][1]; t++) {
          int i = columnwidth[t]-teams[t].length();
          spaces(stream,1+i/2);
          stream.print(teams[t].replace("@","@@"));
          spaces(stream,(i+1)/2);
        }
        stream.println();

        stream.print("                   +");
        for (int t = groups[group][0]; t < groups[group][1]; t++)
          for (int i = columnwidth[t] + 1; i > 0; i--)
            stream.print('-');
        stream.println("-+");

        for (int d = 0; d < ndomains; d++) {
          String domain = domains[d];
          if (domain.length() > 18)
            domain = domain.substring(0, 18-3) + "...";
          stream.print(domain);
          spaces(stream,18 - domain.length() + 1);
          stream.print('|');
          for (int t = groups[group][0]; t < groups[group][1]; t++) {
            stream.print(' ');
            if (matrix[d][t] == TRUE) {
              int i = columnwidth[t]-2;
              spaces(stream,i/2);
              stream.print("[]");
              spaces(stream,(i+1)/2);
            } else if (matrix[d][t] == EXTERNAL) {
              int i = columnwidth[t]-2;
              spaces(stream,i/2);
              stream.print("()");
              spaces(stream,(i+1)/2);
            } else {
              spaces(stream,columnwidth[t]);
            }
          }
          stream.print(' ');
          stream.print('|');
          if (group == ngroups-1) {
            stream.print(' ');
            String s = Integer.toString(total_per_domain[d]);
            spaces(stream,2-s.length());
            stream.print(s);
          }
          stream.println();
        }

        stream.print("                   +");
        for (int t = groups[group][0]; t < groups[group][1]; t++)
          for (int i = columnwidth[t] + 1; i > 0; i--)
            stream.print('-');
        stream.println("-+");

        if (group == ngroups-1) {
          String s = Integer.toString(nteams);
          spaces(stream,4-s.length());
          stream.print(s);
          stream.print(" teams          ");
        } else {
          stream.print("                    ");
        }
        for (int t = groups[group][0]; t < groups[group][1]; t++) {
          int i = columnwidth[t]-teams[t].length();
          spaces(stream,1+i/2);
          stream.print(teams[t].replace("@","@@"));
          spaces(stream,(i+1)/2);
        }
        stream.println();

        if (group == ngroups-1) {
          String s = Integer.toString(ndomains);
          spaces(stream,4-s.length());
          stream.print(s);
          stream.print(" domains        ");
        } else {
          stream.print("                    ");
        }
        for (int t = groups[group][0]; t < groups[group][1]; t++) {
          stream.print(' ');
          String s = Integer.toString(total_per_team[t]);
          int i = columnwidth[t]-s.length();
          int j = (s.length() < 2 ? 1 : 0);
          spaces(stream,(i+j)/2);
          stream.print(s);
          spaces(stream,(i+1-j)/2);
        }
        if (group == ngroups-1) {
          stream.print(' ');
          stream.print(' ');
          String s = Integer.toString(total);
          spaces(stream,3-s.length());
          stream.print(s);
        }
        stream.println();

        stream.println("@end group");
      }
      stream.println("@end example");

      stream.close();
      bf.close();
      f.close();
    } catch (IOException e) {
      e.printStackTrace();
      System.exit(1);
    }
  }

}
