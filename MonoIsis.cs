using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Linq;
using System.Collections.Generic;
using System.Collections;
using System.Threading;
using Isis;

namespace IsisService {
	delegate void insert(string command, int rank);  //Callback for insert
	delegate void query(string command, int rank);   //Callback for get
	
	class IsisServer {
	  	public static int nodeNum;                   //Node number
	  	public static int shardSize;                 //Shard size
	  	public static int myRank;                    //My rank
	  	public static Isis.Group[] shardGroup;       //Group
	  	public static Isis.Timeout timeout;          //Time out
	  	public static int INSERT = 0;                //Insert code
	  	public static int GET = 1;                   //Get code
	  	public static bool isVerbose = true;         //Verbosely output?
	  	public static int is_Started = 0;            //Is ISIS started
	  	public static int memPortNum = 9999;         //memcached port number
	  	
	  	public static void createGroup(int nNum, int sSize, int mRank) {
	  		nodeNum = nNum;
	  		shardSize = sSize;
	  		myRank = mRank;
			
			timeout = new Isis.Timeout(15000, Isis.Timeout.TO_FAILURE);
			
	  		IsisSystem.Start();
	  		if (isVerbose) {
	  			Console.WriteLine("Isis system started!");
	  		}
	  		
//            shardSize = 1;
	  		shardGroup = new Isis.Group[shardSize];
	  		
	  		int groupNum = myRank;
//            groupNum /= sSize;

	  		for (int i = 0; i < shardSize; i++) {
	  			shardGroup[i] = new Isis.Group("group"+groupNum);
	  			
	  			groupNum--;
	  			if (groupNum < 0) {
	  				groupNum += nodeNum;
	  			}
	  		}
	  		
	  		for (int i = 0; i < shardSize; i++) {
	  			int local = i;
	  			
	  			//Insert handler
	  			shardGroup[i].Handlers[INSERT] += (insert)delegate(string command, int rank) {
	  				if (isVerbose) {
	  					Console.WriteLine("Got a command {0}", command);
	  					Console.WriteLine("Rank is {0}", rank);
	  				}
	  				
	  				if (shardGroup[local].GetView().GetMyRank() == rank) {
	  					if (isVerbose) {
	  						Console.WriteLine("Got a message from myself!");
	  					}
	  					shardGroup[local].Reply("Yes");
	  				} else {
	  					string ret = talkToMem(command, INSERT);
	  					if (ret == "STORED") {
	  						shardGroup[local].Reply("Yes");
	  					} else {
	  						shardGroup[local].Reply("No");
	  					}
	  				}
	  			};
	  			
	  			//Get handler
	  			shardGroup[i].Handlers[GET] += (query)delegate(string command, int rank) {
	  				if (isVerbose) {
	  					Console.WriteLine("Got a command {0}", command);
	  				}
  					if (shardGroup[local].GetView().GetMyRank() == rank) {
  						if (isVerbose) {
  							Console.WriteLine("Got a message from myself!");
  						}
  						shardGroup[local].Reply("END\r\n"); //Definitely not presented in local memcached!
  					} else {
  						string ret = talkToMem(command, GET);
  						shardGroup[local].Reply(ret);
  					}
	  			};
	  			
	  			//View handler
	  			shardGroup[i].ViewHandlers += (Isis.ViewHandler)delegate(View v) {
	  				if (isVerbose) {
	  					Console.WriteLine("Got a new view {0}" + v);
	  					Console.WriteLine("Group {0} has {1} members", local, shardGroup[local].GetView().GetSize());
	  				}
	  			};
	  		}
	  		
	  		for (int i = 0; i < shardSize; i++) {
	  			shardGroup[i].Join();
	  		}
	  		
	  		is_Started = 1;
	  		IsisSystem.WaitForever();
	  	}
	  	
	  	public static int commandSend(string command) {
	  		List<string> replyList = new List<string>();
			int	nr = shardGroup[0].Query(Isis.Group.ALL, timeout, INSERT, command, shardGroup[0].GetView().GetMyRank(), new EOLMarker(), replyList);
			
			foreach (string s in replyList) {
				if (s == "Yes") {
					return 0;
				}
			}
			return 1;
	  	}
	  	
	  	public static int isStarted() {
	  		return is_Started;
	  	}
	  	
	  	public static void setVerbose(bool isVerbose) {
	  		IsisServer.isVerbose = isVerbose;
	  	}
	  	
	  	//Talk to local memcached
	  	public static string talkToMem(string command, int commandType) {
	  		TcpClient client = new TcpClient();
	  		string line = "";
	  		string reply = "";
	  		
	  		try {
	  			client.Connect("localhost", memPortNum);
	  			NetworkStream ns = client.GetStream();
	  			byte[] sendBytes = Encoding.ASCII.GetBytes(command);
	  			StreamReader reader = new StreamReader(client.GetStream(), System.Text.Encoding.ASCII);
	  			
	  			//Send command to local memcached
	  			if (ns.CanRead && ns.CanWrite) {
	  				ns.Write(sendBytes, 0, sendBytes.Length);
	  			}
	  			
	  			if (commandType == IsisServer.INSERT) {
  					line = reader.ReadLine();
  					reply = line;
  					client.Close();
	  			} else if (commandType == IsisServer.GET) {
  					while ((line = reader.ReadLine()) != null) {
  						reply += line;
  						reply += "\r\n";
  						
  						if (line == "END") {
  							break;
  						}
  					}
  					client.Close();
	  			}
	  		} catch (Exception e) {
	  			Console.WriteLine("Exception in talking to memcached!");
	  			if (client != null) {
	  				client.Close();
	  			}
	  		}	
	  			return reply;
	  	}
	  	
	  	public static void Main(string[] args) {
//		    createGroup(1, 1, 0);
	  	}
	  		
	}
}

