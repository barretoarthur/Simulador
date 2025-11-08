import java.io.*;
import java.util.*;

public class Comp {
    public static void main(String[] args) {
        try{
            Scanner doc1 = new Scanner(new File(args[0]));
            Scanner doc2 = new Scanner(new File(args[1]));
            int line=1;
            int cont=line;
            while (doc1.hasNext() && doc2.hasNext()){
                String inst1 = doc1.next();
                String inst2 = doc2.next();
                if(!inst1.equals(inst2)){
                    System.out.println("Diferenca encontrada na linha " + line + "\nNo token " + cont);
                    doc1.close();
                    doc2.close();
                    return;
                }
                if(cont%3==0) {
                    line++;
                    cont =1;
                } else{
                    cont++;
                }
            }

            if (doc1.hasNext() || doc2.hasNext()){
                System.out.println("Arquivos com tamanhos diferentes.");
                doc1.close();
                doc2.close();
                return;
            }

            System.out.println("Arquivos iguais.");
            doc1.close();
            doc2.close();
        } catch (FileNotFoundException e) {
            System.out.println(e.getMessage());
        }
    }
}
