from django.db import models
from django.utils import timezone
import datetime

# Create your models here.

class Emails(models.Model):
	
	email = models.EmailField()
	date_added = models.DateTimeField(default=timezone.now)
	
	def __str__(self):
		return self.email

