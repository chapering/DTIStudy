## first R script due to unavailablitiy of the proprietary software - SAS

########################################################################
################   DATA PREPARATION ####################################
########################################################################
# whole raw dataset
alldata <- read.table('./dataR.txt', header=T, sep=' ')

# see the columns
names(alldata)

# saving variable reference afterwards
attach(alldata)

# take a look to verify if the loading is correct
summary(alldata)

# remove all participants' first task record which is confused by training&practice time for p1 and p2
data1 <- alldata[! ((Torder%%10) == 1),]

# remove the first task record which is confused by training&practice time, of p1 and p2 only
data2 <-  alldata[! ((Pid==1 | Pid==2 ) & (Torder%%10) == 1), ]

detach(alldata)

# remove task records with wrong answer
correctdata1 <- data1[! (data1$Correct==0), ]
correctdata2 <- data2[! (data2$Correct==0), ]

# Task-specific records
task1data <- data1[ data1$Tid==1, ]
task2data <- data1[ data1$Tid==2, ]
task3data <- data1[ data1$Tid==3, ]
task4data <- data1[ data1$Tid==4, ]
task5data <- data1[ data1$Tid==5, ]
task6data <- data1[ data1$Tid==6, ]
task7data <- data1[ data1$Tid==7, ]

# data excluding p3 (who is suspected as a semi-expert)
datanop3 <- data1[ ! (data1$Pid==3), ]

########################################################################
################   STATISTICS ANALYSES #################################
########################################################################
std_err <- function(x) sd(x)/sqrt(length(x))

########################################################################
## Basic statistics on the Task completion time
# Time mean per res per tid
attach(data1)

time_meanbreakdown <- tapply(Time, list(Res,Tid), mean)
print (time_meanbreakdown)

# Time standard deviation per res per tid
time_sdbreakdown <- tapply(Time, list(Res,Tid), sd)
print (time_sdbreakdown)

# Time mean standard error per res per tid
time_msebreakdown <- tapply(Time, list(Res,Tid), std_err)
print (time_msebreakdown)

########################################################################
## Basic statistics on the Task correctness
# Correctness mean per res per tid
crt_meanbreakdown <- tapply(Correct, list(Res,Tid), mean)
print ( crt_meanbreakdown )

# Correctness standard deviation per res per tid
crt_sdbreakdown <- tapply(Correct, list(Res,Tid), sd)
print ( crt_sdbreakdown )

# Correctness mean standard error per res per tid
crt_msebreakdown <- tapply(Correct, list(Res,Tid), std_err)
print ( crt_msebreakdown )
detach(data1)

########################################################################
## Correlations
sprintf("overall Time -- Res")
cor(data1$Time, data1$Res)

sprintf("overall Correct -- Res")
cor(data1$Correct, data1$Res)

sprintf("overall Time -- Did")
cor(data1$Time, data1$Did)

sprintf("overall Correct -- Did")
cor(data1$Correct, data1$Did)

for (i in 1:7) {
	cat( sprintf("========= correlation analysis of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]
	cat(sprintf("\t Time ~ Res \n"))
	print( cor(tdata$Time, tdata$Res) )

	cat( sprintf("\t Correct ~ Res \n") )
	print( cor(tdata$Correct, tdata$Res) )

	cat( sprintf("\t Time ~ Data \n") )
	print( cor(tdata$Time, tdata$Did) )

	cat( sprintf("\t Correct ~ Data \n") )
	print( cor(tdata$Correct, tdata$Did) )
}

########################################################################
## Distribution functions
# Emperical distribution function of task completion time
plot.ecdf( data1$Time, main="emperical distribution of task completion time" )

# Emperical distribution function of task completion time
plot.ecdf( data1$Correct, main="emperical distribution of task correctness" )

########################################################################
## Two sample T-test between res in each task
for (i in 1:7) {
	cat( sprintf("========= T-test of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]
	for (n in 1:5) {
		for (m in 1:5) {
			if ( m!=n ) {
				cat( sprintf("\t Res %d ---- Res %d \n", m,n) )
				mdata <- tdata [ tdata$Res == m, ]
				ndata <- tdata [ tdata$Res == n, ]
				print( t.test( mdata$Time, ndata$Time, conf.level=0.95, var.equal=F, alternative='t' ) )
			}
		}	
	}
}

########################################################################
## AVOVA for each task
for (i in 1:7) {
	cat( sprintf("========= AVOVA of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]
	title=sprintf("\t AVOVA of Task %d : Time ~ Res ", i)
	boxplot( tdata$Time~tdata$Res, main=title )
	print( summary( aov( tdata$Time~tdata$Res ) ) )

	title=sprintf("\t AVOVA of Task %d : Correct ~ Res ", i)
	boxplot( tdata$Correct~tdata$Res, main=title )
	print( summary( aov( tdata$Correct~tdata$Res ) ) )
}

########################################################################
## normality checking
qqnorm( data1$Time, main="normality checking : Time"); qqline( data1$Time)
qqnorm( data1$Correct, main="normality checking : Correctness"); qqline( data1$Correct )

########################################################################
## linear regression model for each task
for (i in 1:7) {
	cat( sprintf("========= linear regression model of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]

	title=sprintf("\t linear regression model of Task %d : Time ~ Res ", i)
	tfit <- lm(tdata$Time~task1data$Res)
	print( summary( tfit ) )
	par(mfrow=c(2,2)); plot(tfit, main=title)

	title=sprintf("\t linear regression model of Task %d : Correct ~ Res ", i)
	cfit <- lm(tdata$Correct~task1data$Res)
	print ( summary( cfit ) )
	par(mfrow=c(2,2)); plot(cfit, main=title)
}

### the end.






