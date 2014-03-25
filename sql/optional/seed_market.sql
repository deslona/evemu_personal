-- seeds specified rgion with skills and ships

set @regionid=10000030; -- regionID, 02: The Forge - 01:derelik - 30:heimatar - 16:lonetrek - 42:metropolis - 43:domain
set @saturation=1.0; -- 80% of stations are filled with orders
-- change "categoryID in (16,6)" to categories you want to be seeded

use EVE_Crucible;

-- select stations to fill
create temporary table if not exists tStations (stationId int, solarSystemID int);
truncate table tStations;
select round(count(stationID)*@saturation) into @lim from staStations where regionID=@regionid ;
set @i=0;
insert into tStations 
  select stationID,solarSystemID from staStations where (@i:=@i+1)<=@lim AND regionID=@regionid order by rand();

--actual seeding
INSERT INTO market_orders (typeID, charID, regionID, stationID, bid, price, volEntered, volRemaining, issued, orderState, minVolume, contraband, accountID, duration, isCorp, solarSystemID, escrow, jumps) 
  SELECT typeID,1 as charID, @regionid as regionID, stationID, 0 as bid, IF(basePrice=0, 1000, basePrice/100) as price, 55000000 as volEntered, 55000000 as volRemaining, 130565976636875000 as issued,1 as orderState, 1 as minVolume,0 as contraband, 0 as accountID, 18250 as duration,0 as isCorp, solarSystemID, 0 as escrow, 15 as jumps 
  FROM tStations, invTypes inner join invGroups on invTypes.groupID=invGroups.groupID
  WHERE invTypes.published = 1 and categoryID IN (18, 25);

groupid 450 - 462  catid 25   ores
groupid 34 - 40  catid 18  mins

4, 5, 6, 7, 8, 9, 16, 17, 18, 22, 23, 24, 32, 34, 35, 39, 40, 41, 42, 43, 46)
