from django.shortcuts import render, redirect
from django.contrib.auth.forms import AuthenticationForm
from django.contrib.auth import authenticate, login
from accounts.models import Emails
from django.http import HttpResponse, HttpResponseRedirect
from django.core.mail import send_mail, EmailMessage
from django.urls import reverse
from .forms import CustomUserCreationForm
from django.contrib import messages
from django.views import generic
from django.contrib.auth.decorators import login_required


# Create your views here.
@login_required
def dashboard(request):
    return render(request, 'dashboard/dashboard.html')


def logout_view(request):
    logout(request)
    return render(request, 'landing.html', {})


def register(request):
    if request.method == 'POST':
        f = CustomUserCreationForm(request.POST)
        if f.is_valid():
            f.save()
            messages.success(request, 'Account created successfully')
            # return redirect('register')
            # return HttpResponseRedirect(reverse('login'))
            return render(request, 'registration/registration_success.html', {'form': f})
    else:
        f = CustomUserCreationForm()

    return render(request, 'registration/register.html', {'form': f})


def email(request):
    if request.method == 'POST':
        email = request.POST['email']

    with open('email_body.txt', 'r') as file:
            # create new Emails object
        Emails.objects.create(
            email=email
        )
        email = EmailMessage('MotoGuardian - Thank you!', file.read(), to=[email])
        email.send()

        return HttpResponse('')
