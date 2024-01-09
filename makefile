EE450_Project_ShreyanshDaga:
	gcc -o auc auc.c -lnsl -lpthread -lresolv
	gcc -o client client.c -lnsl -lpthread -lresolv
	

clean:
	rm -f auc client
	
