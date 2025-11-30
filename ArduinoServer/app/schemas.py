from pydantic import BaseModel
import datetime
from typing import Union


class Reading(BaseModel):
    id: int
    timestamp: datetime.datetime
    temp_air: float
    temp_wall: float
    wilg_pow: float
    mech: int
    woda: int
    dym: int
    ogien: int

    class Config:
        orm_mode = True


class ChartDataPoint(BaseModel):
    timestamp: datetime.datetime
    value: Union[float, int, None]

    class Config:
        orm_mode = True
