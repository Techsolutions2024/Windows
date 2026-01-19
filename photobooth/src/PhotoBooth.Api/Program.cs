using PhotoBooth.Core;
using PhotoBooth.Core.Cameras;
using PhotoBooth.Core.Imaging;
using PhotoBooth.Core.Printing;
using PhotoBooth.Core.Sessions;
using PhotoBooth.Api;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy =>
        policy.AllowAnyHeader().AllowAnyMethod().AllowAnyOrigin());
});

builder.Services.AddPhotoBooth(builder.Configuration);

var app = builder.Build();

if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseCors();
app.UseDefaultFiles();
app.UseStaticFiles();

app.MapGet("/api/health", () => Results.Ok(new { ok = true, ts = DateTimeOffset.UtcNow }))
    .WithName("Health")
    .WithOpenApi();

app.MapPhotoBoothApi();

app.Run();
