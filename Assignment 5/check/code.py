import mosspy

userid = 184672036

m = mosspy.Moss(userid, "c")

m.addFile("../NL/mysocket.c")
m.addFile("../vinod.c")

url = m.send()

print ("Report URL: " + url)

m.saveWebPage(url, "submission/report.html")

# Download whole report locally including code diff links
mosspy.download_report(url, "submission/report/", connections=8, log_level=10, on_read=lambda url: print('*', end='', flush=True)) 
