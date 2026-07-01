# OrderMatchingEngineReadMe
order book array for mutliple symbols

map<Prices, Orders> , it iwll be ordered_map as prices should be sorted and in orders will be FIFO , so as the order will  be sorted by timeor queue or <dequeue>
have to prepare two maps , First for BUY and second for SELL side

For buy side we be looking at smallest price first (rbegin()) in the askSide or SELL map ,
 and for sell side we be start from looking at highest price (begin()) in buyside or BUY map

Then do it in chain until order fulfil if order remian will amrk as partial fill and store them in a Ordderbook map , as per the price and quantity and respective to its execution or whether to BUY or SELL

Wait i do have to see if the orderbook stored one or in the map record is sell or buy to correctly execute order, otherwise, if there is only one order of 100 @103 , and another come for sell again to 100 @104,it will gonna see the begin() and execuet that instad of seeing it as buy or sell side 
NOte- > have different map of BUY and SELL side to correctly execute the Orders

thas the basic skeleton i have to prepare first

for skeleton let flow like
accept the incoming order via Acceptorder function which will loop and see for any icoming order and if that order comes
will divert it with the side whether or BUY or SELL to anther function of validate order

Here will see respective to whether sell or buy order, will look in opposite map of that side to execute order , and if anything matches will execute it, 
And if the price we get on first time is 
if wanna sell at 100 @103 and looking in to first of BUY map wee see at 100 @100 and below that like 30 @99 , like the prices too gona be in sort order so.. make no sense to see below if the first one is lower than ask for 
and vice versa for buy side , if askside or SELL map if the price is higher than buying at then no mean to look below , 
will immeditely put that order in the ordeerbook resepctive of bUY or sell side , will insert the order into that [price]
or wil then divert to another fuinction to save in orderbook ()

And if user want to cancel the order will look for the ORderID in the respctiev map and remove that 
for that on time of incoming order will have to encapsulate it with ID and time,

