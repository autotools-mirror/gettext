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
        data.po_files.add(
            new PoFile(
                po.getAttribute("domain").getValue(),
                po.getAttribute("team").getValue(),
                Integer.parseInt(po.getText())));
      }
    }

    // Special treatment of clisp. The percentages are incorrect.
    data.domains.add("clisp");
    if (!data.teams.contains("en"))
      data.teams.add("en");
    data.po_files.add(new PoFile("clisp","en",100));
    data.po_files.add(new PoFile("clisp","de",99));
    data.po_files.add(new PoFile("clisp","fr",99));
    data.po_files.add(new PoFile("clisp","es",90));
    data.po_files.add(new PoFile("clisp","nl",90));

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

      boolean[][] matrix = new boolean[ndomains][];
      for (int d = 0; d < ndomains; d++)
        matrix[d] = new boolean[nteams];
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
            matrix[d][t] = true;
            total_per_domain[d]++;
            total_per_team[t]++;
            total++;
          }
        }
      }

      // Split into separate tables, to keep 80 column width.
      int ngroups;
      int[][] groups;
      if (nteams == 28) {
        ngroups = 2;
        groups = new int[ngroups][];
        groups[0] = new int[] { 0, 15 };
        groups[1] = new int[] { 15, 28 };
      } else {
        ngroups = 1;
        groups = new int[ngroups][];
        groups[0] = new int[] { 0, nteams };
      }

      stream.println("@example");
      for (int group = 0; group < ngroups; group++) {
        if (group > 0)
          stream.println();

        stream.println("@group");

        if (group == 0)
          stream.print("Ready PO files   ");
        else
          stream.print("                 ");
        for (int t = groups[group][0]; t < groups[group][1]; t++)
          stream.print(" "+teams[t]);
        stream.println();

        stream.print("                +");
        for (int t = groups[group][0]; t < groups[group][1]; t++)
          for (int i = teams[t].length() + 1; i > 0; i--)
            stream.print('-');
        stream.println("-+");

        for (int d = 0; d < ndomains; d++) {
          stream.print(domains[d]);
          spaces(stream,16 - domains[d].length());
          stream.print('|');
          for (int t = groups[group][0]; t < groups[group][1]; t++) {
            stream.print(' ');
            if (matrix[d][t]) {
              int i = teams[t].length()-2;
              spaces(stream,i/2);
              stream.print("[]");
              spaces(stream,(i+1)/2);
            } else {
              spaces(stream,teams[t].length());
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

        stream.print("                +");
        for (int t = groups[group][0]; t < groups[group][1]; t++)
          for (int i = teams[t].length() + 1; i > 0; i--)
            stream.print('-');
        stream.println("-+");

        if (group == ngroups-1) {
          String s = Integer.toString(nteams);
          spaces(stream,4-s.length());
          stream.print(s);
          stream.print(" teams       ");
        } else {
          stream.print("                 ");
        }
        for (int t = groups[group][0]; t < groups[group][1]; t++)
          stream.print(" "+teams[t]);
        stream.println();

        if (group == ngroups-1) {
          String s = Integer.toString(ndomains);
          spaces(stream,4-s.length());
          stream.print(s);
          stream.print(" domains     ");
        } else {
          stream.print("                 ");
        }
        for (int t = groups[group][0]; t < groups[group][1]; t++) {
          stream.print(' ');
          String s = Integer.toString(total_per_team[t]);
          int i = teams[t].length()-2;
          spaces(stream,i/2 + (2-s.length()));
          stream.print(s);
          spaces(stream,(i+1)/2);
        }
        stream.print(' ');
        stream.print(' ');
        {
          String s = Integer.toString(total);
          spaces(stream,3-s.length());
          stream.println(s);
        }

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
