from sqlalchemy import Column, Integer, String, DateTime
from sqlalchemy.orm import declarative_base
import datetime

Base = declarative_base()

class Customer(Base):
    __tablename__ = 'customers'
    id = Column(Integer, primary_key=True)
    email = Column(String, unique=True)
    api_key = Column(String, unique=True, nullable=True)
    plan = Column(String, default="default")
    created_at = Column(DateTime, default=datetime.datetime.utcnow)
